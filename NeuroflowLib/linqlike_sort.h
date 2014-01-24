#pragma once

#include "linqlike_base.h"
#include <boost/optional.hpp>
#include <vector>
#include <algorithm>
#include <assert.h>

namespace linqlike
{
    template <typename T>
    struct _sort
    {
        explicit _sort(dir direction) : _direction(direction) { }
        explicit _sort(const T& comparer) : _comparer(comparer) { }

        const boost::optional<T>& comparer() const
        {
            return _comparer;
        }

        const boost::optional<dir> direction() const
        {
            return _direction;
        }

    private:
        boost::optional<T> _comparer;
        boost::optional<dir> _direction;
    };

    inline _sort<int> sort(dir direction)
    {
        return _sort<int>(direction);
    }

    template <typename F>
    auto sort(const F& comparer)
    {
        return _sort<F>(comparer);
    }

    template <typename TColl, typename T = TColl::value_type>
    enumerable<T> operator|(TColl& coll, const _sort<int>& orderBy)
    {
        TColl* pcoll = &coll;
        return enumerable<T>([=]() mutable
        {
            return enumerable<T>::pull_type([=](enumerable<T>::push_type& yield) mutable
            {
                if (orderBy.direction())
                {
                    std::vector<std::reference_wrapper<T>> values;
                    for (auto& v : *pcoll) values.push_back(std::ref(v));

                    if (*orderBy.direction() == dir::asc)
                    {
                        std::sort(values.begin(), values.end());
                    }
                    else
                    {
                        std::sort(values.begin(), values.end(), std::greater<std::reference_wrapper<T>>());
                    }

                    for (auto& v : values)
                    {
                        yield(v);
                    }
                }
            });
        });
    }

    template <typename TColl, typename TComp, typename T = TColl::value_type>
    enumerable<T> operator|(TColl& coll, const _sort<TComp>& orderBy)
    {
        TColl* pcoll = &coll;
        return enumerable<T>([=]() mutable
        {
            return enumerable<T>::pull_type([=](enumerable<T>::push_type& yield) mutable
            {
                if (orderBy.comparer())
                {
                    std::vector<std::reference_wrapper<T>> values;
                    for (auto& v : *pcoll) values.push_back(std::ref(v));
                    auto comp = [=](std::reference_wrapper<T>& v1, std::reference_wrapper<T>& v2) { return (*(orderBy.comparer()))(v1, v2); };
                    std::sort(values.begin(), values.end(), comp);
                    for (auto& v : values)
                    {
                        yield(v);
                    }
                }
            });
        });
    }
}