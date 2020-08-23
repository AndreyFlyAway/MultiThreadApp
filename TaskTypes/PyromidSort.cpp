#include "PyromidSort.h"

static const int RESIZE_VAL = 100;

Pyramid::Pyramid():
	n(0)
{
	data_v.resize(RESIZE_VAL, "");
}

int Pyramid::insert(const std::string s)
{
	if (n == data_v.size())
	{
		data_v.resize(data_v.size() + RESIZE_VAL, "");
	}
	n += 1;
	// TODO: should I?
	data_v[n] = std::move(s);
	bubble_up();

	return 0;
}

void Pyramid::bubble_up()
{
	uint64_t p = parent(n);
	uint64_t c = n;
	if (p == 0)
		return;
	auto compare_val = [&]{return data_v[p].compare(data_v[c]);};
	while ((p != 0) && (compare_val() > 0))
	{
		swap(p, c);
		c = p;
		p = parent(c);
	}
}
