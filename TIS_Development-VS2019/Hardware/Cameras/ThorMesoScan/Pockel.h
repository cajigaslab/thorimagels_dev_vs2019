
class Pockel
{
public:
	Pockel();
	~Pockel();

	long FindPockel();
	long InitPockel();
	bool IsAvaliable();
	long SetLowPower();
	long SetHighPower();
	long MovePockelsToParkPosition();
	long MovePockelsToPowerLevel();

	double MinVoltage;
	double MaxVoltage;

private:
	bool _hasPockel;
};

