/*
 * sortingByOtherValues.h
 *
 *  Created on: 28.06.2016
 *      Author: Yaro
 */

#ifndef SORTINGBYOTHERVALUES_H_
#define SORTINGBYOTHERVALUES_H_

#include <algorithm>
#include <vector>
#include <utility>

template< typename sortedBySelf_type, typename sortByOther_type >
void sortTwoVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, std::vector< sortByOther_type >& sortByOther);

template< typename sortedBySelf_type, typename sortByOther1_type, typename sortByOther2_type >
void sortThreeVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, std::vector< sortByOther1_type >& sortByOther1,
        std::vector< sortByOther2_type >& sortByOther2);

template< typename sortedBySelf_type, typename sortByOther_type >
void nthElementTwoVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, uint32_t n, std::vector< sortByOther_type >& sortByOther);

template< typename sortedBySelf_type, typename sortByOther_type >
void nthElementTwoArraysByFirstArray(sortedBySelf_type* sortBySelf, sortByOther_type* sortByOther, uint32_t n, uint32_t size);

template< typename main_type, typename other_type >
class TwoTupleClass {
public:
    TwoTupleClass(main_type main, other_type other1) :
            main(main), other(other1)
    {
    }

    main_type main;
    other_type other;

    bool operator<(const TwoTupleClass& r) const
            {
        return (this->main < r.main);
    }
};

template< typename sortedBySelf_type, typename sortByOther_type >
void sortTwoVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, std::vector< sortByOther_type >& sortByOther)
{
    std::vector< TwoTupleClass< sortedBySelf_type, sortByOther_type > > tupleVector;
    tupleVector.reserve(sortBySelf.size());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        tupleVector.push_back(
                TwoTupleClass< sortedBySelf_type, sortByOther_type >(sortBySelf[i], sortByOther[i]));
    }

    std::sort(tupleVector.begin(), tupleVector.end());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        sortBySelf[i] = tupleVector[i].main;
        sortByOther[i] = tupleVector[i].other;
    }
}

template< typename main_type, typename other1_type, typename other2_type >
class ThreeTupleClass {
public:
    ThreeTupleClass(main_type main, other1_type other1, other2_type other2) :
            main(main), other1(other1), other2(other2)
    {
    }

    main_type main;
    other1_type other1;
    other2_type other2;

    bool operator<(const ThreeTupleClass& r) const
            {
        return (this->main < r.main);
    }
};

template< typename sortedBySelf_type, typename sortByOther1_type, typename sortByOther2_type >
void sortThreeVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, std::vector< sortByOther1_type >& sortByOther1,
        std::vector< sortByOther2_type >& sortByOther2)
{
    std::vector< ThreeTupleClass< sortedBySelf_type, sortByOther1_type, sortByOther2_type > > tupleVector;
    tupleVector.reserve(sortBySelf.size());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        tupleVector.push_back(
                ThreeTupleClass< sortedBySelf_type, sortByOther1_type, sortByOther2_type >(sortBySelf[i], sortByOther1[i], sortByOther2[i]));
    }

    std::sort(tupleVector.begin(), tupleVector.end());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        sortBySelf[i] = tupleVector[i].main;
        sortByOther1[i] = tupleVector[i].other1;
        sortByOther2[i] = tupleVector[i].other2;
    }
}

template< typename sortedBySelf_type, typename sortByOther_type >
void nthElementTwoVectorsByFirstVector(std::vector< sortedBySelf_type >& sortBySelf, uint32_t n, std::vector< sortByOther_type >& sortByOther)
{
    std::vector< TwoTupleClass< sortedBySelf_type, sortByOther_type > > tupleVector;
    tupleVector.reserve(sortBySelf.size());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        tupleVector.push_back(
                TwoTupleClass< sortedBySelf_type, sortByOther_type >(sortBySelf[i], sortByOther[i]));
    }

    std::nth_element(tupleVector.begin(), tupleVector.begin() + n, tupleVector.end());

    for (uint32_t i = 0; i < sortBySelf.size(); ++i) {
        sortBySelf[i] = tupleVector[i].main;
        sortByOther[i] = tupleVector[i].other;
    }
}

template< typename sortedBySelf_type, typename sortByOther_type >
void nthElementTwoArraysByFirstArray(sortedBySelf_type* sortBySelf, sortByOther_type* sortByOther, uint32_t n, uint32_t size)
{
    std::vector< TwoTupleClass< sortedBySelf_type, sortByOther_type > > tupleVector;
    tupleVector.reserve(size);

    for (uint32_t i = 0; i < size; ++i) {
        tupleVector.push_back(
                TwoTupleClass< sortedBySelf_type, sortByOther_type >(sortBySelf[i], sortByOther[i]));
    }

    std::nth_element(tupleVector.begin(), tupleVector.begin() + n, tupleVector.end());

    for (uint32_t i = 0; i < size; ++i) {
        sortBySelf[i] = tupleVector[i].main;
        sortByOther[i] = tupleVector[i].other;
    }
}

#endif /* SORTINGBYOTHERVALUES_H_ */
