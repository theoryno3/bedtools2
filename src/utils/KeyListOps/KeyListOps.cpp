/*
 * KeyListOps.cpp
 *
 *  Created on: Feb 6, 2014
 *      Author: nek3d
 */

#include "KeyListOps.h"
#include <cfloat>
#include <cmath>
#include <algorithm>

KeyListOps::KeyListOps()
: _keyList(&_nullKeyList),
  _column(1),
  _nullVal("."),
  _delimStr(","),
  _iter(_nullKeyList.begin())
{
	init();

}

KeyListOps::KeyListOps(RecordKeyList *keyList, int column)
: _keyList(keyList),
  _column(column),
  _nullVal("."),
  _delimStr(","),
  _iter(keyList->begin())
{
	init();
}

void KeyListOps::init() {
	_opCodes["sum"] = SUM;
	_opCodes["mean"] = MEAN;
	_opCodes["stddev"] = STDDEV;
	_opCodes["sample_stddev"] = SAMPLE_STDDEV;
	_opCodes["median"] = MEDIAN;
	_opCodes["mode"] = MODE;
	_opCodes["antimode"] = ANTIMODE;
	_opCodes["min"] = MIN;
	_opCodes["max"] = MAX;
	_opCodes["absmin"] = ABSMIN;
	_opCodes["absmax"] = ABSMAX;
	_opCodes["count"] = COUNT;
	_opCodes["distinct"] = DISTINCT;
	_opCodes["count_distinct"] = COUNT_DISTINCT;
	_opCodes["distinct_only"] = DISTINCT_ONLY;
	_opCodes["collapse"] = COLLAPSE;
	_opCodes["concat"] = CONCAT;
	_opCodes["freq_asc"] = FREQ_ASC;
	_opCodes["freq_desc"] = FREQ_DESC;
	_opCodes["first"] = FIRST;
	_opCodes["last"] = LAST;
}


KeyListOps::~KeyListOps() {

}

KeyListOps::OP_TYPES KeyListOps::getOpCode(const QuickString &operation) const {
	//If the operation does not exist, return INVALID.
	//otherwise, return code for given operation.
	map<QuickString, OP_TYPES>::const_iterator iter = _opCodes.find(operation);
	if (iter == _opCodes.end()) {
		return INVALID;
	}
	return iter->second;
}

// return the total of the values in the vector
double KeyListOps::getSum() {
	if (empty()) return NAN;

	double theSum = 0.0;
	for (begin(); !end(); next()) {
		theSum += getColValNum();
	}
	return theSum;
}

// return the average value in the vector
double KeyListOps::getMean() {
	if (empty()) return NAN;

	return getSum() / (float)getCount();
}


 // return the standard deviation
double KeyListOps::getStddev() {
	if (empty()) return NAN;

	double avg = getMean();
	double squareDiffSum = 0.0;
	for (begin(); !end(); next()) {
		double val = getColValNum();
		double diff = val - avg;
		squareDiffSum += diff * diff;
	}
	return squareDiffSum / (float)getCount();
}
// return the standard deviation
double KeyListOps::getSampleStddev() {
	if (empty()) return NAN;

	double avg = getMean();
	double squareDiffSum = 0.0;
	for (begin(); !end(); next()) {
		double val = getColValNum();
		double diff = val - avg;
		squareDiffSum += diff * diff;
	}
	return  squareDiffSum / ((float)getCount() - 1.0);
}

// return the median value in the vector
double KeyListOps::getMedian() {
	if (empty()) return NAN;

	//get sorted vector. if even number of elems, return middle val.
	//if odd, average of two.
	toArray(true, ASC);
	size_t count = getCount();
	if (count % 2) {
		//odd number of elements. Take middle one.
		return _numArray[count/2];
	} else {
		//even numnber of elements. Take average of middle 2.
		double sum = _numArray[count/2 -1] + _numArray[count/2];
		return sum / 2.0;
	}
}

// return the most common value in the vector
const QuickString &KeyListOps::getMode() {
	if (empty()) return _nullVal;

	makeFreqMap();

	//now pass through the freq map and keep track of which key has the highest occurance.
	freqMapType::iterator maxIter = _freqMap.begin();
	int maxVal = 0;
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		if (_freqIter->second > maxVal) {
			maxIter = _freqIter;
			maxVal = _freqIter->second;
		}
	}
	_retStr = maxIter->first;
	return _retStr;
}
// return the least common value in the vector
const QuickString &KeyListOps::getAntiMode() {
	if (empty()) return _nullVal;

	makeFreqMap();

	//now pass through the freq map and keep track of which key has the highest occurance.
	freqMapType::iterator minIter = _freqMap.begin();
	int minVal = INT_MAX;
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		if (_freqIter->second < minVal) {
			minIter = _freqIter;
			minVal = _freqIter->second;
		}
	}
	_retStr =  minIter->first;
	return _retStr;
}
// return the minimum element of the vector
double KeyListOps::getMin() {
	if (empty()) return NAN;

	double minVal = DBL_MAX;
	for (begin(); !end(); next()) {
		double currVal = getColValNum();
		minVal = (currVal < minVal) ? currVal : minVal;
	}
	return  minVal;
}

// return the maximum element of the vector
double KeyListOps::getMax() {
	if (empty()) return NAN;

	double maxVal = DBL_MIN;
	for (begin(); !end(); next()) {
		double currVal = getColValNum();
		maxVal = (currVal > maxVal) ? currVal : maxVal;
	}
	return maxVal;
}

// return the minimum absolute value of the vector
double KeyListOps::getAbsMin() {
	if (empty()) return NAN;

	double minVal = DBL_MAX;
	for (begin(); !end(); next()) {
		double currVal = abs(getColValNum());
		minVal = (currVal < minVal) ? currVal : minVal;
	}
	return minVal;
}
// return the maximum absolute value of the vector
double KeyListOps::getAbsMax() {
	if (empty()) return NAN;

	double maxVal = DBL_MIN;
	for (begin(); !end(); next()) {
		double currVal = abs(getColValNum());
		maxVal = (currVal > maxVal) ? currVal : maxVal;
	}
	return maxVal;
}
// return the count of element in the vector
uint32_t KeyListOps::getCount() {
	return _keyList->size();
}
// return a delimited list of the unique elements
const QuickString &KeyListOps::getDistinct() {
	if (empty()) return _nullVal;
	// separated list of unique values. If something repeats, only report once.
	makeFreqMap();
	_retStr.clear();
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		if (_freqIter != _freqMap.begin()) _retStr += _delimStr;
		_retStr.append(_freqIter->first);
	}
	return _retStr;
}

const QuickString &KeyListOps::getDistinctOnly() {
	if (empty()) return _nullVal;

	//separated list of only unique values. If item repeats, discard.
	makeFreqMap();
	_retStr.clear();
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		if (_freqIter->second != 1) continue;
		if (_freqIter != _freqMap.begin()) _retStr += _delimStr;
		_retStr.append(_freqIter->first);
	}
	return _retStr;
}

// return a the count of _unique_ elements in the vector
uint32_t KeyListOps::getCountDistinct() {
	if (empty()) return 0;

	makeFreqMap();
	return _freqMap.size();
}
// return a delimiter-separated list of elements
const QuickString &KeyListOps::getCollapse(const QuickString &delimiter) {
	if (empty()) return _nullVal;

	//just put all items in one big separated list.
	_retStr.clear();
	int i=0;
	for (begin(); !end(); next()) {
		if (i > 0) _retStr += _delimStr;
		_retStr.append(getColVal());
		i++;
	}
	return _retStr;

}
// return a concatenation of all elements in the vector
const QuickString &KeyListOps::getConcat() {
	if (empty()) return _nullVal;

	//like collapse but w/o commas. Just a true concat of all vals.
	//just swap out the delimChar with '' and call collapse, then
	//restore the delimChar.
	QuickString oldDelimStr(_delimStr);
	_delimStr = "";
	getCollapse(); //this will store it's results in the _retStr method.
	_delimStr = oldDelimStr;
	return _retStr;
}

// return a histogram of values and their freqs. in desc. order of frequency
const QuickString &KeyListOps::getFreqDesc() {
	if (empty()) return _nullVal;

	//for each uniq val, report # occurances, in desc order.
	makeFreqMap();
	//put freq map into multimap where key is the freq and val is the item. In other words, basically a reverse freq map.
	histDescType hist;
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		hist.insert(pair<int, QuickString>(_freqIter->second, _freqIter->first));
	}
	//now iterate through the reverse map we just made and output it's pairs in val:key format.
	_retStr.clear();
	for (histDescType::iterator histIter = hist.begin(); histIter != hist.end(); histIter++) {
		if (histIter != hist.begin()) _retStr += _delimStr;
		_retStr.append(histIter->second);
		_retStr += ":";
		_retStr.append(histIter->first);
	}
	return _retStr;
}
// return a histogram of values and their freqs. in asc. order of frequency
const QuickString &KeyListOps::getFreqAsc() {
	if (empty()) return _nullVal;

	//for each uniq val, report # occurances, in asc order.
	makeFreqMap();
	//put freq map into multimap where key is the freq and val is the item. In other words, basically a reverse freq map.
	histAscType hist;
	for (; _freqIter != _freqMap.end(); _freqIter++) {
		hist.insert(pair<int, QuickString>(_freqIter->second, _freqIter->first));
//		hist[*(_freqIter->second)] = _freqIter->first;
	}
	//now iterate through the reverse map we just made and output it's pairs in val:key format.
	_retStr.clear();
	for (histAscType::iterator histIter = hist.begin(); histIter != hist.end(); histIter++) {
		if (histIter != hist.begin()) _retStr += _delimStr;
		_retStr.append(histIter->second);
		_retStr += ":";
		_retStr.append(histIter->first);
	}
	return _retStr;
}
// return the first value in the list
const QuickString &KeyListOps::getFirst() {
	if (empty()) return _nullVal;

	//just the first item.
	begin();
	return getColVal();
}
// return the last value in the list
const QuickString &KeyListOps::getLast() {
	if (empty()) return _nullVal;

	//just the last item.
	begin();
	for (size_t i = 0; i < getCount() -1; i++) {
		next();
	}
	return getColVal();
}

const QuickString &KeyListOps::getColVal() {
	return _iter->value()->getField(_column);
}

double KeyListOps::getColValNum() {
	return atof(_iter->value()->getField(_column).c_str());
}

void KeyListOps::toArray(bool useNum, SORT_TYPE sortVal) {

	//TBD: optimize performance with better memory management.
	if (useNum) {
		_numArray.resize(_keyList->size());
		int i=0;
		for (begin(); !end(); next()) {
			_numArray[i] = getColValNum();
			i++;
		}
	} else {
		_qsArray.resize(_keyList->size());
		int i=0;
		for (begin(); !end(); next()) {
			_qsArray[i] = getColVal();
			i++;
		}
	}
	if (sortVal != UNSORTED) {
		sortArray(useNum, sortVal == ASC);
	}
}

void KeyListOps::sortArray(bool useNum, bool ascOrder)
{
	if (useNum) {
		if (ascOrder) {
			sort(_numArray.begin(), _numArray.end(), less<double>());
		} else {
			sort(_numArray.begin(), _numArray.end(), greater<double>());
		}
	} else {
		if (ascOrder) {
			sort(_qsArray.begin(), _qsArray.end(), less<QuickString>());
		} else {
			sort(_qsArray.begin(), _qsArray.end(), greater<QuickString>());
		}
	}
}

void KeyListOps::makeFreqMap() {
	_freqMap.clear();

	//make a map of values to their number of times occuring.
	for (begin(); !end(); next()) {
		_freqMap[getColVal()]++;
	}
	_freqIter = _freqMap.begin();
}
