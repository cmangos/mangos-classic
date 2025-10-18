#pragma once
#include <cstdarg>
#include <string>
#include <iosfwd>
#include <set>
#include <list>
#include <map>

namespace ai
{
    class Qualified
    {
    public:
        Qualified() {};
        Qualified(const std::string& qualifier) : qualifier(qualifier) {}
        Qualified(int32 qualifier1) { Qualify(qualifier1); }

    public:
        virtual void Qualify(int32 qualifier) { std::ostringstream out; out << qualifier; this->qualifier = out.str(); }
        virtual void Qualify(const std::string& qualifier) { this->qualifier = qualifier; }
        std::string getQualifier() const { return qualifier; }
        void Reset() { qualifier.clear(); }

        static std::string MultiQualify(const std::vector<std::string>& qualifiers, const std::string& separator, const std::string_view brackets = "{}")
        { 
            std::stringstream out;
            for (uint8 i = 0; i < qualifiers.size(); i++)
            {
                const std::string& qualifier = qualifiers[i];
                if (i == qualifiers.size() - 1)
                {
                    out << qualifier;
                }
                else
                {
                    out << qualifier << separator;
                }
            }

            if (brackets.empty())
            {
                return out.str();
            }
            else
            {
                return brackets[0] + out.str() + brackets[1];
            }
        }

        static std::vector<std::string> getMultiQualifiers(const std::string& qualifier1, const std::string& separator, const std::string_view brackets = "{}")
        { 
            std::vector<std::string> result;

            std::string view = qualifier1;

            if(view.find(brackets[0]) == 0)
                view = qualifier1.substr(1, qualifier1.size()-2);

            size_t last = 0; 
            size_t next = 0; 

            if (view.find(brackets[0]) == std::string::npos)
            {
                while ((next = view.find(separator, last)) != std::string::npos)
                {

                    result.push_back((std::string)view.substr(last, next - last));
                    last = next + separator.length();
                }

                result.push_back(view.substr(last));
            }
            else
            {
                int8 level = 0;
                std::string sub;
                while (next < view.size() || level < 0)
                {
                    if (view[next] == brackets[0])
                        level++;
                    else if (view[next] == brackets[1])
                        level--;
                    else if (!level && view.substr(next, separator.size()) == separator)
                    {
                        result.push_back(sub);
                        sub.clear();
                        next += separator.size();
                        continue;
                    }
                    
                    sub += view[next];

                    next++;
                }

                result.push_back(sub);
            }

            return result;
        }

        static bool isValidNumberString(const std::string& str)
        {
            bool valid = !str.empty();
            if (valid)
            {
                // Check for sign character at the beginning
                size_t start = 0;
                if (str[0] == '+' || str[0] == '-')
                {
                    start = 1;
                }

                // Loop through each character to check if it's a digit
                for (size_t i = start; i < str.size(); ++i) 
                {
                    if (!std::isdigit(str[i])) 
                    {
                        // Non-numeric character found
                        valid = false;
                        break;
                    }
                }
            }

            return valid;
        }
        
        static int32 getMultiQualifierInt(const std::string& qualifier1, uint32 pos, const std::string& separator)
        { 
            std::vector<std::string> qualifiers = getMultiQualifiers(qualifier1, separator);
            if (qualifiers.size() > pos && isValidNumberString(qualifiers[pos]))
            {
                return stoi(qualifiers[pos]);
            }

            return 0;
        }

        static std::string getMultiQualifierStr(const std::string& qualifier1, uint32 pos, const std::string& separator)
        { 
            std::vector<std::string> qualifiers = getMultiQualifiers(qualifier1, separator);
            return (qualifiers.size() > pos) ? qualifiers[pos] : "";
        }
    
    protected:
        std::string qualifier;
    };

    template <class T> class NamedObjectFactory
    {
    protected:
        using ActionCreator = std::function<T* (PlayerbotAI* ai)>;
        std::map<std::string, ActionCreator> creators;

    public:
        T* create(const std::string& inName, PlayerbotAI* ai)
        {
            std::string name = inName;
            size_t found = name.find("::");
            std::string qualifier;
            if (found != std::string::npos)
            {
                qualifier = name.substr(found + 2);
                name = name.substr(0, found);
            }

            if (creators.find(name) == creators.end())
            {
                return NULL;
            }

            ActionCreator& creator = creators[name];

            T* object = creator(ai);
            Qualified *q = dynamic_cast<Qualified *>(object);
            if (q && found != std::string::npos)
            {
                q->Qualify(qualifier);
            }

            return object;
        }

        std::set<std::string> supports()
        {
            std::set<std::string> keys;
            for (typename std::map<std::string, ActionCreator>::iterator it = creators.begin(); it != creators.end(); it++)
            {
                keys.insert(it->first);
            }

            return keys;
        }
    };


    template <class T> class NamedObjectContext : public NamedObjectFactory<T>
    {
    public:
        NamedObjectContext(bool shared = false, bool supportsSiblings = false) :
            NamedObjectFactory<T>(), shared(shared), supportsSiblings(supportsSiblings) {}

        T* create(std::string name, PlayerbotAI* ai)
        {
            if (created.find(name) == created.end())
                return created[name] = NamedObjectFactory<T>::create(name, ai);

            return created[name];
        }

        virtual ~NamedObjectContext()
        {
            Clear();
        }

        void Clear()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }

            created.clear();
        }

        void Erase(const std::string& name)
        {
            if (created.find(name) != created.end())
            {
                delete created[name];
                created.erase(name);
            }
        }

        void Update()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Update();
            }
        }

        void Reset()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Reset();
            }
        }

        bool IsShared() { return shared; }
        bool IsSupportsSiblings() { return supportsSiblings; }

        bool IsCreated(const std::string& name) { return created.find(name) != created.end(); }

        std::set<std::string> GetCreated()
        {
            std::set<std::string> keys;
            for (typename std::map<std::string, T*>::iterator it = created.begin(); it != created.end(); it++)
                keys.insert(it->first);
            return keys;
        }

    protected:
        std::map<std::string, T*> created;
        bool shared;
        bool supportsSiblings;
    };

    template <class T> class NamedObjectContextList
    {
    public:
        virtual ~NamedObjectContextList()
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                NamedObjectContext<T>* context = *i;
                if (!context->IsShared())
                    delete context;
            }
        }

        void Add(NamedObjectContext<T>* context)
        {
            contexts.push_back(context);
        }

        T* GetObject(const std::string& name, PlayerbotAI* ai)
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                T* object = (*i)->create(name, ai);
                if (object) return object;
            }
            return NULL;
        }

        void Update()
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsShared())
                    (*i)->Update();
            }
        }

        void Reset()
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                (*i)->Reset();
            }
        }

        std::set<std::string> GetSiblings(const std::string& name)
        {
            std::set<std::string> siblings;
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if ((*i)->IsSupportsSiblings())
                {
                    std::set<std::string> supported = (*i)->supports();
                    std::set<std::string>::iterator found = supported.find(name);
                    if (found != supported.end())
                    {
                        supported.erase(found);
                        siblings.insert(supported.begin(), supported.end());
                    }
                }
            }

            return siblings;
        }

        std::set<std::string> supports()
        {
            std::set<std::string> result;

            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> supported = (*i)->supports();

                for (std::set<std::string>::iterator j = supported.begin(); j != supported.end(); j++)
                    result.insert(*j);
            }
            return result;
        }

        bool IsCreated(const std::string& name)
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if ((*i)->IsCreated(name))
                    return true;
            }
            return false;
        }

        std::set<std::string> GetCreated()
        {
            std::set<std::string> result;

            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> createdKeys = (*i)->GetCreated();

                for (std::set<std::string>::iterator j = createdKeys.begin(); j != createdKeys.end(); j++)
                    result.insert(*j);
            }
            return result;
        }

        void Erase(const std::string& name)
        {
            for (typename std::list<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                (*i)->Erase(name);
            }
        }

    private:
        std::list<NamedObjectContext<T>*> contexts;
    };

    template <class T> class NamedObjectFactoryList
    {
    public:
        virtual ~NamedObjectFactoryList()
        {
            for (typename std::list<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
                delete *i;
        }

        void Add(NamedObjectFactory<T>* context)
        {
            factories.push_front(context);
        }

        T* GetObject(const std::string& name, PlayerbotAI* ai)
        {
            for (typename std::list<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
            {
                T* object = (*i)->create(name, ai);
                if (object) return object;
            }
            return NULL;
        }

    private:
        std::list<NamedObjectFactory<T>*> factories;
    };
};
