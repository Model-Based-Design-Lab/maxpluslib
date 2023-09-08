/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   smpls.h
 *
 *  Author			:	Alireza Mohammadkhani (a.mohammadkhani@tue.nl)
 *
 *  Date            :   January 4, 2021
 *
 *  Function        :   Switching max plus linear system.
 *
 *  History         :
 *      04-01-21    :   Initial version.
 *      18-01-21    :   consistency check on IOA of SMPLS with events.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */
#include "../../base/fsm/fsm.h"
#include "../../maxplus/graph/mpautomaton.h"
#include "../../fsmsadf/analysis/throughput/thrutils.h"

#include <sstream>

using namespace FSM;

namespace MaxPlus
{
	class SMPLS
	{
	public:
		ScenarioMatrices *sm = new ScenarioMatrices();
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
		list<Matrix *> *eventRows;

		DissectedScenarioMatrix()
		{
			core = new ScenarioMatrices();
			eventRows = new list<Matrix *>();
		}
	};
	typedef InputAction Mode;
	typedef OutputAction Event;
	typedef CString EventOutcome;

	class SMPLSwithEvents : public SMPLS
	{
	public:
		list<pair<Mode, Event>> *sigma = new list<pair<Mode, Event>>();					//relation between mode and event
		list<pair<Event, EventOutcome>> *gamma = new list<pair<Event, EventOutcome>>(); //relation between event and outcome

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
		void addToSigma(pair<Mode *, Event *> *p)
		{
		}
		void addToGamma(pair<Event *, EventOutcome *> *p)
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
		list<DissectedScenarioMatrix *> *disMatrices = new list<DissectedScenarioMatrix *>();
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
		void prepareMatrices(IOAState *s, multiset<Event> *eventList, IOASetOfEdges *visitedEdges);

		DissectedScenarioMatrix *findDissectedScenarioMatrix(CString sName);

		/**
		 * recursive part of isConsistent
		 */
		void isConsistentUtil(IOAState *s, list<Event> *eventList, IOASetOfStates *finalStates, CString *errMsg, map<IOAState *, list<Event> *> *visited);
		 
		  
		void determinizeUtil(IOAState* s, IOASetOfStates* visited, IOASetOfStates* finalStates, CString* errMsg, ofstream& outfile);

		bool compareEventLists(list<Event> *l1, list<Event> *l2);

		void dissectScenarioMatrices();
	};

}
