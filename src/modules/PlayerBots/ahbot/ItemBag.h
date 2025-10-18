#pragma once
#include "Category.h"
#if defined(MANGOSBOT_TWO) || MAX_EXPANSION == 2
#include "Util/Util.h"
#else
#include "Util/Util.h"
#endif

namespace ahbot
{
    class CategoryList
    {
    public:
        CategoryList();
        virtual ~CategoryList();

        Category* operator[](int index) { return categories[index]; }
        int32 size() { return categories.size(); }
        static CategoryList instance;

    private:
        void Add(Category* category);

    private:
        std::vector<Category*> categories;
    };

    template<class T>
    void Shuffle(std::vector<T>& items) 
    {
        uint32 count = items.size();
        for (uint32 i = 0; i < count * 5; i++)
        {
            int i1 = urand(0, count - 1);
            int i2 = urand(0, count - 1);

            T item = items[i1];
            items[i1] = items[i2];
            items[i2] = item;
        }
    }

    class ItemBag 
    {
    public:
        ItemBag();

    public:
        void Init(bool silent = false);
        std::vector<uint32>& Get(Category* category) { return content[category]; }
        int32 GetCount(Category* category) { return content[category].size(); }
        int32 GetCount(Category* category, uint32 item);
        bool Add(ItemPrototype const* proto);

    protected:
        virtual void Load() = 0;
        virtual std::string GetName() = 0;

    protected:
        std::map<Category*, std::vector<uint32> > content;
    };

    class AvailableItemsBag : public ItemBag
    {
    public:
        AvailableItemsBag() {}

    protected:
        virtual void Load();
        virtual std::string GetName() { return "available"; }
    };

    class InAuctionItemsBag : public ItemBag
    {
    public:
        InAuctionItemsBag(uint32 auctionId) : auctionId(auctionId) {}

    protected:
        virtual void Load();
        virtual std::string GetName();

    private:
        uint32 auctionId;
    };
};
