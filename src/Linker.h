#ifndef LINKER_H_
#define LINKER_H_

#include "ILinkable.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>

class Linker final {
public:
	static int64_t NextId;

	template <typename T>
	void Add(std::shared_ptr<T> p)
	{
		int64_t id = NextId++;
		p->SetId(id);
		m_linkables[typeid(T)].emplace(id, p);
	}

	template <typename T>
	void Remove(int64_t id)
	{
		m_linkables[typeid(T)].erase(id);
	}

	template <typename T>
	std::shared_ptr<T> Get(int64_t id)
	{
		return std::static_pointer_cast<T>(m_linkables[typeid(T)][id]);
	}

	template <typename T>
	std::vector<std::shared_ptr<T>> GetAll()
	{
		auto& m = m_linkables[typeid(T)];
		std::vector<std::shared_ptr<T>> v;
		std::transform(m.begin(), m.end(), std::back_inserter(v), [](auto& pair) { return std::static_pointer_cast<T>(pair.second); });
		return v;
	}

private:
	std::unordered_map<std::type_index, std::unordered_map<int64_t, std::shared_ptr<ILinkable>>> m_linkables;
};

#endif // LINKER_H_