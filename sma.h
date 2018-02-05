#ifndef __SIMPLE_MOVING_AVERAGE_H__
#define __SIMPLE_MOVING_AVERAGE_H__

/** This is a lean implentation of a simple moving average
 *  with configurable depth.
 */

template <typename T>
class simpleAverage {
public:
	/** Construct a smaAverage with depth _depth */
	simpleAverage (int _depth = 1) : depth(_depth) {}
	/** Construct a smaAverage with depth _depth and initial value _init */
	simpleAverage (int _depth, const T _init) : depth(_depth), avg(_init) {}
	/** cast to template type opetrator */
	inline operator const T& ()
	{
		return avg;
	}
	/** change depth of the sma */
	inline void setDepth(int _depth)
	{
		depth = _depth;
	}
	/** add a new value to and update the average value */
	inline simpleAverage<T> & update(const T wert)
	{
		avg *= (depth-1);
		avg += wert;
		avg /= depth;
		return *this;
	}
	inline simpleAverage<T> & operator=(const T rhs)
	{
		avg = rhs;
		return *this;
	}
private:
	int depth;
	T avg;
};

#endif  /* __SIMPLE_MOVING_AVERAGE_H__ */
