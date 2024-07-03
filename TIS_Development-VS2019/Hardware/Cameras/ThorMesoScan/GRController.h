class GRController
{
public:
	GRController();
	~GRController();

	long StartGR();
	long StopGR();
	bool IsAvaliable();

	long MoveGalvoToParkPosition();
	long MoveGalvoToPostion(double pos);

private:
	bool _hasGRController;
};

