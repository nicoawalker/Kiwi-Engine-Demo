#ifndef _TASKREWARD_H_
#define _TASKREWARD_H_

class TaskReward
{
protected:

	double m_xpReward;

public:

	TaskReward():
		m_xpReward( 0 )
	{ }

	void SetXP( double reward ) { m_xpReward = reward; }

	double GetXP()const { return m_xpReward; }

};

#endif