// define general public functions

inline double round(double number)
{
	return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}
