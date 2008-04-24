#include "chipmunk.h"

class ChipmunkHelper
{
public:
	static ChipmunkHelper *Instance();
	void reInstance();
	~ChipmunkHelper() {};
	cpSpace *getSpace();
	void setSpace(cpSpace* s);

	cpBody *getStaticBody();
	cpBody *getFrontWheel();
	cpBody *getBackWheel();
	void setStaticBody(cpBody *body);
	void setFrontWheel(cpBody *body);
	void setBackWheel(cpBody *body);

private:
	static ChipmunkHelper *mp_instance;
	void initPhysics();
	cpSpace *m_space;
	cpBody *m_body;
        cpBody *m_ab;
        cpBody *m_af;
	cpBody *m_wb;
	cpBody *m_wf;
	
};

