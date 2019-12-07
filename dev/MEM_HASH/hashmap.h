#pragma once

#include <cstdint>
#include <list>
#include <vector>
#include <functional>
#include <stdexcept>
#include "../LRUCACHE/LRUcache.h"
#define CACHE_SIZE 1024 * 1024 / 8
template <typename K, typename V>
class hashmap {
public:
    /**
     *  @param  capacity    ��ʼ����
     *  @param  load_factor �������ӣ�Ĭ��0.75
     *  @note   ��ʵ��Ԫ����������capacity * load_factorʱ�������rehash
     */
    LRUCache* mycache = new LRUCache(CACHE_SIZE);
    int mem_insert_count = 0;
    int cache_insert_count = 0;
    int mem_erase_count = 0;
    int cache_erase_count = 0;
    int mem_lookup_count = 0;
    int cache_lookup_count = 0;

    hashmap(int capacity, double load_factor = 0.75);

    virtual ~hashmap();

    /**
     *  ���һ��key�Ƿ����
     *  @param  key
     *  @return ������true������false
     */
    bool contain(const K &key);

    /**
     *  ��ȡֵ
     *  @param  key
     *  @return key��Ӧ��value��������������׳�logic_error�쳣
     */
    V& get(const K &key);

    /**
     *  ����ֵ
     *  @param  key
     *  @param  value
     *  @note   �ڲ��������key��value��һ������
     */
    void set(const K &key, const V &value);

    /**
     *  �Ƴ�һ���ֵ��
     *  @param  key
     *  @return key��Ӧ��ֵ��������������׳�logic_error�쳣
     */
    V remove(const K &key);

private:
    /** �ڲ����ڴ洢һ����ֵ�� */
    struct entry {
        K key;
        V value;
    };

    /** �ڲ����飨slots�������� */
    typedef std::vector<std::list<entry>> slots_t;

private:
    /** �ж�key�Ƿ����ھ����� */
    bool is_key_belongs_old(const K &key);

    /**
     *  ��һ���ڲ������л�ȡ��ֵ��
     *  @param  slots   һ���ڲ�����
     *  @param  key
     *  @return ��ֵ�ԣ����������׳�logic_error�쳣
     */
    entry& get_from_slots(slots_t &slots, const K &key);

    void move_into_slots(slots_t &slots, entry &&e);

    /**
     *  ��������ɾ��һ����ֵ��
     *  @param  slots   һ���ڲ�����
     *  @param  key
     *  @return key��Ӧ�ļ�ֵ�ԣ�������������׳�logic_error�쳣
     */
    entry remove_from_slots(slots_t &slots, const K &key);

    /** ��ʼ����rehash */
    void start_rehash();

    /** ��������rehash */
    void finish_rehash();

    /** ����һ��slotִ��rehash */
    void rehash_next_slot();

private:
    /** �Ѵ洢����Ԫ������ */
    int total_entries;

    /** �������� */
    double load_factor;

    /** �ڲ����飬0�ǵ�ǰ���飬1�Ǳ��þ����� */
    slots_t double_slots[2];
    constexpr static int slots_current = 0;
    constexpr static int slots_old = 1;

    /** �Ƿ������ڽ���ʽrehash������ */
    bool is_rehashing;

    /** double_slots[slots_old]��δrehash��ʣ�ಿ��slot����ʼ����������ʽrehash���ȣ� */
    int remain_old_index;

    /** hash���� */
    std::hash<K> hasher;
};


template<typename K, typename V>
hashmap<K, V>::hashmap(int capacity, double load_factor)
    :total_entries(0), load_factor(load_factor), is_rehashing(false), remain_old_index(0) {
    double_slots[slots_current].resize(capacity);
}

template<typename K, typename V>
hashmap<K, V>::~hashmap() {}

template<typename K, typename V>
bool
hashmap<K, V>::contain(const K &key) {
    try {
        this->get_from_slots(this->double_slots[this->is_key_belongs_old(key)], key);
        return true;
    } catch(std::logic_error) {
    }
    return false;
}

template<typename K, typename V>
V&
hashmap<K, V>::get(const K &key) {
    // ������ڽ���ʽrehash����ô����һ��rehash
    if (this->is_rehashing) {
        this->rehash_next_slot();
    }

    return this->get_from_slots(this->double_slots[this->is_key_belongs_old(key)], key).value;
}

template<typename K, typename V>
void
hashmap<K, V>::set(const K &key, const V &value) {
    // ������ڽ���ʽrehash����ô����һ��rehash
    if (this->is_rehashing) {
        this->rehash_next_slot();
    }

    // �¼�ֵ�Դ���
    this->move_into_slots(this->double_slots[this->is_key_belongs_old(key)], {key, value});
}

template<typename K, typename V>
V
hashmap<K, V>::remove(const K &key) {
    // ������ڽ���ʽrehash����ô����һ��rehash
    if (this->is_rehashing) {
        this->rehash_next_slot();
    }

        return this->remove_from_slots(this->double_slots[this->is_key_belongs_old(key)], key).value;
}

template<typename K, typename V>
bool
hashmap<K, V>::is_key_belongs_old(const K &key) {
    // ����ʽrehash�����У����key�ھ������е���������remain_old_index֮����ô˵����key���洢�ھ�������
    return this->is_rehashing && (int)(this->hasher(key) % this->double_slots[slots_old].size()) >= this->remain_old_index;
}

template<typename K, typename V>
typename hashmap<K, V>::entry&
hashmap<K, V>::get_from_slots(slots_t &slots, const K &key) {
    auto index = this->hasher(key) % slots.size();
    for (auto &e : slots[index]) {
        if(!mycache->refer(&e.key))
            mem_lookup_count++;
        else
            cache_lookup_count++;
        if (e.key == key) {
            return e;
        }
    }
    throw std::logic_error("no entry for key");
}

template<typename K, typename V>
void
hashmap<K, V>::move_into_slots(slots_t &slots, entry &&e) {
    if(!mycache->refer(&e.key))
        mem_lookup_count++;
    else
        cache_lookup_count++;
    auto index = this->hasher(e.key) % slots.size();
    auto &slot = slots[index];
    for (auto &old : slot) {
        if(!mycache->refer(&old.key))
            mem_lookup_count++;
        else
            cache_lookup_count++;
        if (old.key == e.key) {
            old.value = std::move(e.value);
            return;
        }
    }
    slot.push_front(std::move(e));

    ++this->total_entries;

    // ����������������Ҫ����rehash�������������rehash����ʱ�������µġ�
    if (!this->is_rehashing && this->total_entries >= this->double_slots[slots_current].size() * this->load_factor) {
        this->start_rehash();
    }
}

template<typename K, typename V>
typename hashmap<K, V>::entry
hashmap<K, V>::remove_from_slots(slots_t &slots, const K &key) {
    auto index = this->hasher(key) % slots.size();
    auto &slot = slots[index];
    for (auto iter = slot.begin(); iter != slot.end(); ++iter) {
        if (iter->key == key) {
            if(!mycache->refer(&iter->key))
                mem_lookup_count++;
            else
                cache_lookup_count++;
            auto old = std::move(*iter);
            slot.erase(iter);
            --this->total_entries;
            return old;
        }
    }
    throw std::logic_error("no entry for key");
}

template<typename K, typename V>
void
hashmap<K, V>::start_rehash() {
    this->is_rehashing = true;

    // �ѵ�ǰ����ȫ������ȫ���ƶ������������Դ�rehash
    this->double_slots[slots_old] = std::move(this->double_slots[slots_current]);
    this->remain_old_index = 0;
    for(auto it = double_slots->begin(); it != double_slots->end(); it++) {
        if(!mycache->refer(&it))
            mem_lookup_count++;
        else
            cache_lookup_count++;
    }

    // ��ǰ��������
    this->double_slots[slots_current].resize(this->total_entries * 2);
}

template<typename K, typename V>
void
hashmap<K, V>::finish_rehash() {
    this->is_rehashing = false;
    this->double_slots[slots_old] = slots_t(); // �����ͷ�vector
    this->remain_old_index = 0;
}

template<typename K, typename V>
void
hashmap<K, V>::rehash_next_slot() {
    for (; this->remain_old_index < (int)this->double_slots[slots_old].size(); ++this->remain_old_index) {
        if (!this->double_slots[slots_old][this->remain_old_index].empty()) {
            break;
        }
    }

    if (this->remain_old_index < (int)this->double_slots[slots_old].size()) {
        auto& slot = this->double_slots[slots_old][this->remain_old_index];
        // ��һ����slot�е�����Ԫ��ȫ��rehash���µĵ�ǰ������
        for (auto &e : slot) {
            if(!mycache->refer(&e))
                mem_lookup_count++;
            else
                cache_lookup_count++;
            this->move_into_slots(this->double_slots[slots_current], std::move(e));
        }
        slot.clear();

        ++this->remain_old_index;
    } else {
        // �������Ѿ�ȫ��rehash���
        this->finish_rehash();
    }
}
