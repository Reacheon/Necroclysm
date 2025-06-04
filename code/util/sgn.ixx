export module sgn;

export int sgn(int val)
{
	if (val == 0)
	{
		return 0;
	}
	else if (val > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}