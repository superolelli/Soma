#include "Buff.hpp"





void Buff::SetStandardValues()
{
	stats.Reset();
	isPositive = true;
	onSelf = false;
	duration = 0;
}



void Buff::ApplySkill(const Buff &_buff)
{
	stats += _buff.stats;

	if(duration <= 0)
		isPositive = _buff.isPositive;

	onSelf = _buff.onSelf;
	duration += _buff.duration;
}