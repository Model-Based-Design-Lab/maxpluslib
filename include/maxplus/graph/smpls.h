#include "base/fsm/fsm.h"
#include "base/fsm/iofsm.h"
#include "graph/mpautomaton.h"
#include "algebra/mpmatrix.h"
#include <sstream>
#include <map>



using namespace FSM;

// TODO: change name to SMPLS terminology
typedef std::map<CString, std::shared_ptr<MaxPlus::Matrix>> ScenarioMatrices;


class EdgeLabeledScenarioFSM : public
	::FSM::Labeled::FiniteStateMachine<CId, CString>
{
public:
	virtual ~EdgeLabeledScenarioFSM() {};
	virtual void removeDanglingStates();
};


namespace MaxPlus
{
	class SMPLS
	{
	public:
		std::shared_ptr<ScenarioMatrices> sm = std::make_shared<ScenarioMatrices>();

		EdgeLabeledScenarioFSM *elsFSM = new EdgeLabeledScenarioFSM();

		MaxPlusAutomaton *convertToMaxPlusAutomaton();
		void loadAutomatonFromIOAFile(CString fileName);
		/*
		Loads max-plus matrices from an mpt file into sm
		*/
		void loadMPMatricesFromMPTFile(CString file);
		void loadAutomatonFromDispatchingFile(CString fileName);
// transposes all matrices of the SMPLS
		void transposeMatrices();
		/*
		Produces POOSL output from SMPLS
		*/
		void convertToPoosl(CString file);

	};

	class DissectedScenarioMatrix
	{
	public:
		ScenarioMatrices *core;
		std::list<std::shared_ptr<Matrix>> *eventRows;

		DissectedScenarioMatrix()
		{
			core = new ScenarioMatrices();
			eventRows = new std::list<std::shared_ptr<Matrix>>();
		}
	};
	typedef InputAction Mode;
	typedef OutputAction Event;
	typedef CString EventOutcome;

	class SMPLSwithEvents : public SMPLS
	{
	public:
		std::list<std::pair<Mode, Event>> *sigma = new std::list<std::pair<Mode, Event>>();					//relation between mode and event
		std::list<std::pair<Event, EventOutcome>> *gamma = new std::list<std::pair<Event, EventOutcome>>(); //relation between event and outcome

		IOAutomaton *ioa;

		SMPLSwithEvents()
		{
			this->ioa = new IOAutomaton();
		}

		SMPLSwithEvents(IOAutomaton *ioa)
		{
			this->ioa = ioa;
		}
		// TODO: control the way sigma and gamma are filled
		void addToSigma(std::pair<Mode *, Event *> *p)
		{
		}
		void addToGamma(std::pair<Event *, EventOutcome *> *p)
		{
		}

		/**
		 * checks the consistency rules for SMPLS with events
		 */
		bool isConsistent();

		void saveDeterminizedIOAtoFile(CString file);

		/**
		 * creates a max-plus automaton from SMPLS with events
		 */
		MaxPlusAutomaton *convertToMaxPlusAutomaton();

		/*
		Loads an IOAutomaton file and stores in elsFMS
		*/
		void loadIOAutomatonFromIOAFile(CString fileName);

		void loadIOAutomatonFromDispatchingFile(CString fileName);
		/*
		Loads events from an activity file and fills gamma and sigma
		*/
		void loadActivities(CString file);


		/*
		* goes through the gamma relation and finds the event of the outcome
		*/
		Event findEventByOutcome(EventOutcome outcome);
	private:
		std::list<DissectedScenarioMatrix *> *disMatrices = new std::list<DissectedScenarioMatrix *>();
		uint numberOfResources = 0;
		uint biggestMatrixSize = 0;

		/*
		* after we generate the matrices through 'preapreMatrices', we have to make them
		* all square (to the size of the biggest matrix) by adding rows and cols of -infty
		*/
		void makeMatricesSquare();



		/*
		* goes through the sigma relation and finds the event emitted by mode
		*/
		Event findEventByMode(Mode mode);

		/**
		 * smpls with events needs to prepare the matrices to be able to perform analysis.
		 * this includes adding rows and columns of -inf and 0 based on the spec
		 * allowing the system to analyze processing or conveying event timings
		 */
		void prepareMatrices(IOAState& s, std::multiset<Event> *eventList, IOASetOfEdges *visitedEdges);

		DissectedScenarioMatrix *findDissectedScenarioMatrix(CString sName);

		/**
		 * recursive part of isConsistent
		 */
		void isConsistentUtil(IOAState& s, std::list<Event> *eventList, const IOASetOfStates& finalStates, CString *errMsg, std::map<IOAState *, std::list<Event> *> *visited);


		void determinizeUtil(IOAState& s, IOASetOfStates* visited, const IOASetOfStates& finalStates, CString* errMsg, std::ofstream& outfile);

		bool compareEventLists(std::list<Event> *l1, std::list<Event> *l2);

		void dissectScenarioMatrices();
	};

}
