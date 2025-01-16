#pragma once

#include <typeindex>

#include "EASTL/unique_ptr.h"
#include "EASTL/hash_map.h"
#include "EASTL/fixed_function.h"
#include "EASTL/vector.h"

namespace Orin::Overkill
{
    // Allocates memory in 16 aligned chuncks
	// No move is involved
	class EventsStorage
	{
		template<typename T>
		struct MallocDeleter
		{
			void operator()(T* p) const
			{
				::_aligned_free(p);
			}
		};

		static inline char *DoAllocate(size_t bytes, size_t alignment = 16)
		{
			return (char*)::_aligned_malloc(bytes, alignment);
		}

		static inline char *DoReallocate(char* ptr, size_t bytes, size_t alignment = 16)
		{
			return (char*)::_aligned_realloc(ptr, bytes, alignment);
		}

		static const uint32_t MAX_CHUNK_SIZE = 4096;

		using Chunk = eastl::unique_ptr<char, MallocDeleter<char>>;
		eastl::vector<Chunk> chunks;

		uint32_t topChunkId   = 0;
		uint32_t topChunkSize = 0;

	public:
		char* Allocate(uint32_t sz, uint32_t alignment = 16)
		{
			const uint32_t szAligned = (sz + (alignment - 1)) & ~(alignment - 1);
			if (topChunkSize == 0 || topChunkSize + szAligned > MAX_CHUNK_SIZE)
			{
				topChunkId   = (uint32_t)chunks.size();
				topChunkSize = 0;
				chunks.emplace_back(DoAllocate(MAX_CHUNK_SIZE, alignment));
			}

			const uint32_t offset = topChunkSize;
			topChunkSize += szAligned;

			return chunks[topChunkId].get() + offset;
		}

		void Clear()
		{
			topChunkId   = 0;
		 	topChunkSize = 0;

			chunks.clear();
		}
	};

	struct EventsQueue
	{
		EventsStorage storage;

		using EventId  = size_t;
		using EventPtr = char*;
        using EventRecord = eastl::pair<EventId, EventPtr>;
		eastl::vector<EventRecord> events;
		eastl::vector<eastl::fixed_function<sizeof(void*), void()>> eventDtors;

        using HandlerObject = void*;
		using HandlerMethod = eastl::fixed_function<2 * sizeof(void*), void(EventPtr)>;
        using Handler = eastl::pair<HandlerObject, HandlerMethod>;
		eastl::hash_map<EventId, eastl::vector<Handler>> handlers;

		bool inDispatch = false;

		EventsQueue() = default;
		EventsQueue(const EventsQueue &) = delete;

		void operator=(const EventsQueue &) = delete;

		// TODO: Add move ctor. Moving a queue is okay.
		EventsQueue(EventsQueue &&) = delete;
		void operator=(EventsQueue &&) = delete;

		~EventsQueue()
		{
			Clear();
		}

		void ClearEvents()
		{
			for (const auto &dtor : eventDtors)
			{
				dtor();
			}
			events.clear();
			eventDtors.clear();
			storage.Clear();
		}

		void Clear()
		{
			ClearEvents();
            handlers.clear();
		}

		template<typename Event>
		void PushEvent(Event&& inEvent)
		{
			if (inDispatch)
			{
				InvokeEvent(eastl::move(inEvent));
				return;
			}

			const EventId id = std::type_index(typeid(Event)).hash_code();
			Event *event = new (storage.Allocate(sizeof(Event))) Event(eastl::move(inEvent));

			events.push_back(eastl::make_pair(id, (EventPtr)event));
			eventDtors.emplace_back([event]() { event->~Event(); });
		}

		template<typename Event, typename... Args>
		void PushEvent(Args&&... args)
		{
			if (inDispatch)
			{
				InvokeEvent(eastl::forward<Args>(args)...);
				return;
			}

			const EventId id = std::type_index(typeid(Event)).hash_code();
			Event *event = new (storage.Allocate(sizeof(Event))) Event(eastl::forward<Args>(args)...);

			events.push_back(eastl::make_pair(id, (EventPtr)event));
			eventDtors.emplace_back([event]() { event->~Event(); });
		}

		template<typename Event>
		void InvokeEvent(Event&& event)
		{
			auto res = handlers.find(std::type_index(typeid(Event)).hash_code());
			if (res != handlers.end())
			{
				for (const auto &handler : res->second)
				{
					handler.second((EventPtr)&event);
				}
			}
		}

		template<typename Event, typename... Args>
		void InvokeEvent(Args&&... args)
		{
			InvokeEvent(Event(eastl::forward<Args>(args)...));
		}

		template<typename Event, typename Object, typename Method>
		void Subscribe(Object *object, Method method)
		{
			auto res = handlers.insert(std::type_index(typeid(Event)).hash_code());
			res.first->second.emplace_back(eastl::make_pair((HandlerObject)object, [object, method](EventPtr evt)
			{
				(object->*method)(*(const Event *)evt);
			}));
		}

        void Unsubscribe(HandlerObject object)
		{
            for (auto &kv : handlers)
            {
                for (int i = (int)kv.second.size() - 1; i >= 0; --i)
                {
                    if (kv.second[i].first == object)
                    {
                        kv.second.erase(kv.second.begin() + i);
                    }
                }
            }
        }

		void Dispatch()
		{
			inDispatch = true;

			for (const EventRecord &evt : events)
			{
				auto res = handlers.find(evt.first);
				if (res != handlers.end())
				{
					for (const auto &handler : res->second)
					{
						handler.second(evt.second);
					}
				}
			}

			inDispatch = false;

			ClearEvents();
		}
	};

	struct EventSet
	{
		virtual void Subscribe(EventsQueue::HandlerObject object) const = 0;
	};

	template<typename HandlerType, typename EventType, void (HandlerType::*Method)(const EventType&)>
	struct EventDescription
	{
		static void Subscribe(EventsQueue::HandlerObject object)
		{
			MainEventsQueue::Subscribe<EventType>((HandlerType*)object, Method);
		}
	};

	template<typename ObjectType, typename... Descs>
	struct EventSetDetails final : EventSet
	{
		static constexpr size_t Count = sizeof...(Descs);

		template<typename... Args>
		struct Unpack;

		template<>
		struct Unpack<>
		{
			static void Subscribe(ObjectType *object) {}
		};

		template<typename Head, typename... Tail>
		struct Unpack<Head, Tail...>
		{
			static void Subscribe(ObjectType *object)
			{
				Head::Subscribe(object);
				Unpack<Tail...>::Subscribe(object);
			}
		};

		void Subscribe(EventsQueue::HandlerObject object) const override
		{
			Unpack<Descs...>::Subscribe((ObjectType*)object);
		}
	};

	#define ORIN_EVENTS(Class) static const EventSetDetails<Class
	#define ORIN_EVENT(Class, Event, Method) ,EventDescription<Class, Event, &Class::Method>
	#define ORIN_EVENTS_END(Class) > Class##EventSet;

	#define ORIN_EVENTS_SUBSCRIBE(Class) Class##EventSet.Subscribe(this);
	#define ORIN_EVENTS_UNSUBSCRIBE() MainEventsQueue::Unsubscribe(this);
}