import devicearraypool;
import devicearray;
import std.algorithm;
import nfhelpers;

struct DeviceArrayGroup
{
	this(DeviceArrayPool pool)
	{
		assert(pool);

		this._pool = pool;
	}

	DeviceArray add(size_t idx, size_t size)
	{
		if (idx >= _arrays.length) _arrays.length = idx + 1;
		return _arrays[idx] = _pool.createArray(size);
	}

	DeviceArray get(size_t idx) 
	{
		return _arrays[idx];
	}

	bool tryGet(size_t idx, out DeviceArray result)
	{
		if (idx < _arrays.length && _arrays[idx])
		{
			result = _arrays[idx];
			return true;
		}
		return false;
	}

	void zero()
	{
		if (_arrays.length > 0 && _pool.isAllocated()) _pool.zero();
	}

	@property size_t size() 
	{
		return _arrays.filter!(a => a).map!(a => a.size()).nfSum;
	}

	@property auto arrays() 
	{
		return _arrays.filter!(a => a);
	}

	private DeviceArrayPool _pool;

	private DeviceArray[] _arrays;
}