#ifdef ASTRA_CUDA

#include "astra/CudaSartAlgorithm.h"

#include "astra/cuda/2d/sart.h"

#include "astra/Logging.h"

using namespace std;
using namespace astra;

//----------------------------------------------------------------------------------------
// Constructor
CCudaSartAlgorithm::CCudaSartAlgorithm()
{
    m_bIsInitialized = false;
    CCudaReconstructionAlgorithm2D::_clear();
}

//----------------------------------------------------------------------------------------
// Destructor
CCudaSartAlgorithm::~CCudaSartAlgorithm()
{
    // The actual work is done by ~CCudaReconstructionAlgorithm2D
}

//---------------------------------------------------------------------------------------
// Initialize - Config
bool CCudaSartAlgorithm::initialize(const Config& _cfg)
{
	ConfigReader<CAlgorithm> CR("CudaSartAlgorithm", this, _cfg);

    m_bIsInitialized = CCudaReconstructionAlgorithm2D::initialize(_cfg);

    if (!m_bIsInitialized)
        return false;

    sart = new astraCUDA::SART();

    m_pAlgo = sart;
    m_bAlgoInit = false;

	if (CR.hasOption("SinogramMaskId")) {
		ASTRA_CONFIG_CHECK(false, "SART_CUDA", "Sinogram mask option is not supported.");
	}

	// projection order
	int projectionCount = m_pSinogram->getGeometry().getProjectionAngleCount();
	std::vector<int> projectionOrder;
	std::string projOrder;
	if (!CR.getOptionString("ProjectionOrder", projOrder, "random"))
		return false;
	if (projOrder == "sequential") {
		projectionOrder.resize(projectionCount);
		for (int i = 0; i < projectionCount; i++) {
			projectionOrder[i] = i;
		}
		sart->setProjectionOrder(&projectionOrder[0], projectionCount);
	} else if (projOrder == "random") {
		projectionOrder.resize(projectionCount);
		for (int i = 0; i < projectionCount; i++) {
			projectionOrder[i] = i;
		}
		for (int i = 0; i < projectionCount-1; i++) {
			int k = (rand() % (projectionCount - i));
			int t = projectionOrder[i];
			projectionOrder[i] = projectionOrder[i + k];
			projectionOrder[i + k] = t;
		}
		sart->setProjectionOrder(&projectionOrder[0], projectionCount);
	} else if (projOrder == "custom") {
		if (!CR.getOptionIntArray("ProjectionOrderList", projectionOrder))
			return false;
		sart->setProjectionOrder(&projectionOrder[0], projectionOrder.size());
	} else {
		ASTRA_ERROR("Unknown ProjectionOrder");
		return false;
	}

	if (!CR.getOptionNumerical("Relaxation", m_fLambda, 1.0f))
		return false;

    return true;
}

//---------------------------------------------------------------------------------------
// Initialize
bool CCudaSartAlgorithm::initialize(CProjector2D* _pProjector,
                                         CFloat32ProjectionData2D* _pSinogram,
                                         CFloat32VolumeData2D* _pReconstruction)
{
    m_bIsInitialized = CCudaReconstructionAlgorithm2D::initialize(_pProjector, _pSinogram, _pReconstruction);

    if (!m_bIsInitialized)
        return false;
    
    m_fLambda = 1.0f;

    sart = new astraCUDA::SART();
    
    m_pAlgo = sart;
    m_bAlgoInit = false;
    
    return true;
}

//----------------------------------------------------------------------------------------

void CCudaSartAlgorithm::updateProjOrder(string& projOrder)
{
    // projection order
    int projectionCount = m_pSinogram->getGeometry().getProjectionAngleCount();
    int* projectionOrder = NULL;
    
    if (projOrder == "sequential") {
        projectionOrder = new int[projectionCount];
        for (int i = 0; i < projectionCount; i++) {
            projectionOrder[i] = i;
        }
        sart->setProjectionOrder(projectionOrder, projectionCount);
        delete[] projectionOrder;
    } else if (projOrder == "random") {
        projectionOrder = new int[projectionCount];
        for (int i = 0; i < projectionCount; i++) {
            projectionOrder[i] = i;
        }
        for (int i = 0; i < projectionCount-1; i++) {
            int k = (rand() % (projectionCount - i));
            int t = projectionOrder[i];
            projectionOrder[i] = projectionOrder[i + k];
            projectionOrder[i + k] = t;
        }
        sart->setProjectionOrder(projectionOrder, projectionCount);
        delete[] projectionOrder;
    }
}




//----------------------------------------------------------------------------------------

void CCudaSartAlgorithm::updateSlice(CFloat32ProjectionData2D* _pSinogram,
                                    CFloat32VolumeData2D* _pReconstruction)
{
    m_pSinogram = _pSinogram;
    m_pReconstruction = _pReconstruction;
}

//----------------------------------------------------------------------------------------

void CCudaSartAlgorithm::setRelaxationParameter(float lambda)
{
    m_fLambda = lambda;
    sart->setRelaxation(m_fLambda);
}


//----------------------------------------------------------------------------------------

void CCudaSartAlgorithm::initCUDAAlgorithm()
{
    CCudaReconstructionAlgorithm2D::initCUDAAlgorithm();

    astraCUDA::SART* pSart = dynamic_cast<astraCUDA::SART*>(m_pAlgo);

    pSart->setRelaxation(m_fLambda);
}


#endif // ASTRA_CUDA
