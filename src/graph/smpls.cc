#include "maxplus/base/fsm/fsm.h"
#include "graph/smpls.h"
#include "algebra/mptype.h"
#include "maxplus/base/fsm/iofsm.h"
#include "maxplus/graph/mpautomaton.h"
#include <memory>

using namespace FSM;

using ELSEdge = ::FSM::Labeled::Edge<CId, CString>;
using ELSEdgeRef = ::FSM::Labeled::EdgeRef<CId, CString>;
using ELSState = ::FSM::Labeled::State<CId, CString>;
using ELSStateRef = ::FSM::Labeled::StateRef<CId, CString>;
using ELSSetOfStates = ::FSM::Labeled::SetOfStates<CId, CString>;
using ELSSetOfEdges = ::FSM::Abstract::SetOfEdges;
using ELSSetOfEdgeRefs = ::FSM::Abstract::SetOfEdgeRefs;
using ELSSetOfStateRefs = ::FSM::Abstract::SetOfStateRefs;

namespace MaxPlus::SMPLS {

    // destructor is put deliberately into the cc source to ensure the class vtable is accessible
    // see: <https://stackoverflow.com/questions/3065154/undefined-reference-to-vtable>

	EdgeLabeledModeFSM::~EdgeLabeledModeFSM(){}

    /*removes states of the fsm with no outgoing edges.*/
	// TODO (Marc Geilen) carefully check and test and move to fsm
    void EdgeLabeledModeFSM::removeDanglingStates()
    {

        // temporary sets of states and edges
        ELSSetOfEdgeRefs edgesToBeRemoved;
        ELSSetOfStateRefs statesToBeRemoved;

        // const ELSSetOfStates& elsStates = this->getStates();
        auto& elsEdges = dynamic_cast<ELSSetOfEdges&>(this->getEdges());


        /*go through all edges and find all edges that end in
        dangling states. Also store dangling states.*/
        for (const auto& it: elsEdges)
        {

            auto e = dynamic_cast<ELSEdgeRef>(&(*it.second));
            const auto& s = dynamic_cast<ELSStateRef>(e->getDestination());
            const auto& oEdges = dynamic_cast<const ELSSetOfEdges&>(s->getOutgoingEdges());
            if (oEdges.empty())
            {
                edgesToBeRemoved.insert(e);
                statesToBeRemoved.insert(s);
            }
        }

        while (!edgesToBeRemoved.empty())
        {

            //remove dangling states
            for (const auto *srIt: statesToBeRemoved)
            {
				this->removeState(dynamic_cast<const ELSState&>(*srIt));
            }

            //remove edges ending in dangling states
            //remove edges ending in dangling states from the outgoing edges of their source states
            for (const auto *erIt: edgesToBeRemoved) {
                const auto& e = dynamic_cast<const ELSEdge&>(*erIt);
                this->removeEdge(e);
				const auto *const s = dynamic_cast<ELSStateRef>(e.getSource());

				const auto *ms = dynamic_cast<ELSStateRef>(s);
                this->removeEdge(e);
            }

            //empty the temporary sets
            edgesToBeRemoved.clear();
            statesToBeRemoved.clear();

            // elsStates = this->getStates();
            elsEdges = this->getEdges();

            /*go through all edges and find all edges that end in
            dangling states. Also store dangling states.*/
            for (const auto& it: elsEdges)
            {

                auto e = dynamic_cast<ELSEdgeRef>(&(*(it.second)));
                const auto s = dynamic_cast<ELSStateRef>(e->getDestination());
                const auto& oEdges = dynamic_cast<const ELSSetOfEdges&>(s->getOutgoingEdges());
                if (oEdges.empty())
                {
                    edgesToBeRemoved.insert(e);
                    statesToBeRemoved.insert(s);
                }
            }
        }
    }

	/**
	 * returns the largest finite element of a row up to and including colNumber
	 */
	MPTime getMaxOfRowUntilCol(Matrix& M, uint rowNumber, uint colNumber)
	{
		if (rowNumber > M.getRows()) {
			throw CException("Matrix getMaxOfRow input row index out of bounds.");
		}
		if (colNumber > M.getCols()) {
			throw CException("Matrix getMaxOfRowUntilCol input col index out of bounds.");
		}
    	MPTime largestEl = MP_MINUS_INFINITY;
		for (unsigned int c = 0; c < colNumber; c++) {
			largestEl = MP_MAX(largestEl, M.get(rowNumber, c));
		}
		return largestEl;
	}

	/**
	 * returns the largest finite element of a row up to and including colNumber
	 */
	MPTime getMaxOfColUntilRow(Matrix& M, uint colNumber, uint rowNumber)
	{
		if (rowNumber > M.getRows()) {
			throw CException("Matrix getMaxOfRow input row index out of bounds.");
		}
		if (colNumber > M.getCols()) {
			throw CException("Matrix getMaxOfRowUntilCol input col index out of bounds.");
		}
    	MPTime largestEl = MP_MINUS_INFINITY;
		for (unsigned int r = 0; r < rowNumber; r++) {
			largestEl = MP_MAX(largestEl, M.get(r, colNumber));
		}
		return largestEl;
	}


	/*
	* Loads an entity from its starting '{' to the corresponding '}'
	*/
	std::string loadEntity(std::ifstream& stream, std::string line)
	{
		std::string out = line;
		size_t openBracketCount = 0;
		size_t closeBracketCount = 0;
		openBracketCount += std::count(line.begin(), line.end(), '{');
		closeBracketCount += std::count(line.begin(), line.end(), '}');

		// get the whole activity {} into a string
		while (getline(stream, line))
		{

			openBracketCount += std::count(line.begin(), line.end(), '{');
			closeBracketCount += std::count(line.begin(), line.end(), '}');
			out += line;

			if (openBracketCount == closeBracketCount)
			{
				openBracketCount = 0;
				closeBracketCount = 0;
				break;
			}
		}
		return out;
	}
	std::shared_ptr<MaxPlusAutomaton> SMPLS::convertToMaxPlusAutomaton() const
	{
		auto mpa = std::make_shared<MaxPlusAutomaton>();
		//transposeMatrices();
		// create the FSM states for every pair of a states of the FSM
		// and an initial token

		const auto& I = dynamic_cast<const ELSSetOfStateRefs&>(this->elsFSM.getInitialStates());
		const auto& F = dynamic_cast<const ELSSetOfStateRefs&>(this->elsFSM.getFinalStates());
		const auto& Q = dynamic_cast<const ELSSetOfStates&>(this->elsFSM.getStates());
		for (const auto& q: Q)
		{
            auto& qq = *(q.second);
			const auto& e = qq.getOutgoingEdges();
			unsigned int nrTokens = 0;
			if (! e.empty())
			{
				CString label = (dynamic_cast<ELSEdgeRef>(*e.begin()))->getLabel();

				for (const auto& smIt: this->sm)
				{
					if ((smIt).first == label)
					{
						nrTokens = (smIt).second->getCols();
						break;
					}
				}
			}
			else
			{
				nrTokens = (*this->sm.begin()).second->getCols();
			}
			// create a state for (q, k)
			const CId qId = (dynamic_cast<ELSState&>(qq)).getLabel();
			bool isInitial = false;
			bool isFinal = false;
			// if els state is initial
			for (const auto& i: I)
			{
				CId iId = (dynamic_cast<const ELSState&>(*i)).getLabel();
				if (iId == qId)
				{
					isInitial = true;
					break;
				}
			}
			// if els state is final
			for (const auto& f: F)
			{
				CId fId = (dynamic_cast<const ELSState&>(*f)).getLabel();
				if (fId == qId)
				{
					isFinal = true;
					break;
				}
			}

			// create the states needed per transition matrix
			for (unsigned int k = 0; k < nrTokens; k++)
			{
				MPAState* s = mpa->addState(makeMPAStateLabel(qId, k));
				//std::cout << "DEBUG adding state to mpa id: " << (CString)(s->getLabel().id) << ", tkn: " <<  (s->getLabel().tokenNr)<< std::endl;
				if (isInitial) {
					mpa->addInitialState(*s);
				}
				if (isFinal) {
					mpa->addFinalState(*s);
				}
			}
		}
		// add the edges. For every edge (q1, q2) in the original fsm,
		// let s be the mode of q2 and let M_s be the matrix of s
		// For every non -inf element d=M_s(k,m) add an edge from state (q1,k)
		// to (q2,m) labelled with d.

		// for every state of the fsm...
		for (const auto& q: Q)
		{
			auto& q1 = dynamic_cast<ELSState&>(*(q.second));
			CId q1Id = q1.getLabel();

			// for every outgoing edge of the state
			const auto& t = dynamic_cast<const ELSSetOfEdgeRefs&>(q1.getOutgoingEdges());
			for (const auto *e: t)
			{
				const auto *tr = dynamic_cast<ELSEdgeRef>(e);
				CId q2Id = dynamic_cast<ELSStateRef>(tr->getDestination())->getLabel();
				CString sc = tr->getLabel();
				std::shared_ptr<Matrix> Ms = this->sm.at(sc);
				size_t r = Ms->getRows();
				size_t c = Ms->getCols();

				// for every entry in the mode matrix of that edge
				for (size_t row = 0; row < r; row++)
				{
					for (size_t col = 0; col < c; col++)
					{
						const MPDelay d = Ms->get(static_cast<unsigned int>(row), static_cast<unsigned int>(col));

						if (!MP_IS_MINUS_INFINITY(d))
						{
							MPAStateRef src = mpa->getStateLabeled(makeMPAStateLabel(q1Id, static_cast<unsigned int>(col)));
							MPAStateRef dst = mpa->getStateLabeled(makeMPAStateLabel(q2Id, static_cast<unsigned int>(row)));
							MPAEdgeLabel el = makeMPAEdgeLabel(d, sc);
							el.mode = CString(tr->getLabel());
							mpa->addEdge(*src, el, *dst);
						}
					}
				}
			}
		}

		return mpa;
	}

	/* this is not fully functional for now. you can use the version that it prints out
	* it also does not check for input actions that don't belong to the same event, which it should.
	*/
	void SMPLSwithEvents::saveDeterminizedIOAtoFile(const CString& file)
	{
		/**
		 * Deterministic IOA is defined with:
		 *	The I/O automaton has exactly one initial state;
		 *  The final states have no outgoing transition;
		 *	Branches in the I/O automaton are based on different outcomes of the same event;
		 */

		if (this->ioa == nullptr) {
			throw CException("The automaton of smpls is not loaded!");
		}

		std::ofstream outfile(file);
		outfile << "ioautomaton statespace{ \r\n";
		const auto& I = this->ioa->getInitialStates();

		const auto& finalStates = dynamic_cast<const IOASetOfStateRefs&>(this->ioa->getFinalStates());

		CString errMsg = "";
		auto i = I.begin();
		auto& s = dynamic_cast<const IOAState&>(*(*i));
		i++;
		//we remove the rest of the initial states since only one is allowed
		for (; i != I.end();)
		{
			ioa->removeState(dynamic_cast<const IOAState&>(*((*i))));
		}
		IOASetOfStateRefs visitedStates;
		determinizeUtil(s, visitedStates, finalStates, errMsg, outfile);
		outfile << "}";
		outfile.close();
	}

	/**
	* checks the consistency rules for SMPLS with events
	*/

	bool SMPLSwithEvents::isConsistent()
	{
		if (this->ioa == nullptr) {
			throw CException("The automaton of smpls is not loaded!");
		}
		EventList eventList;

		const auto& I = dynamic_cast<const IOASetOfStates&>(this->ioa->getInitialStates());

		const auto& finalStates = dynamic_cast<const IOASetOfStates&>(this->ioa->getFinalStates());

		std::map<IOAStateRef,EventList> visited;
		CString errMsg = "";
		for (const auto& i: I)
		{
			isConsistentUtil((dynamic_cast<IOAState&>(*(i.second))), eventList, finalStates, errMsg, visited);
		}

		if (!errMsg.empty())
		{
			// TODO(mgeilen) no std:cout
			std::cout << "IO Automaton is not consistent, error message: " << std::endl;
			std::cout << errMsg << std::endl;
			return false;
		}

		return true;
	}

	/**
		 * creates a max-plus automaton from SMPLS with events
		 */
	std::shared_ptr<MaxPlusAutomaton> SMPLSwithEvents::convertToMaxPlusAutomaton()
	{
		dissectModeMatrices();
		IOASetOfEdgeRefs visitedEdges;
		std::multiset<Event> eventList;

		// create the elsFsm with the same state structure but we change the matrices in a depth-first search
		const auto& I = dynamic_cast<const IOASetOfStates&>(this->ioa->getStates());
		for (const auto& i: I)
		{
			this->elsFSM.addState(dynamic_cast<ELSState&>(*(i.second)).getLabel());
		}

		const auto& I2 = dynamic_cast<const IOASetOfStates&>(this->ioa->getInitialStates());
		for (const auto& i:  I2)
		{
			prepareMatrices((dynamic_cast<IOAState&>(*(i.second))), eventList, visitedEdges);
			this->elsFSM.addInitialState((dynamic_cast<ELSState&>(*(i.second))));
		}
		const auto& I3 = dynamic_cast<const IOASetOfStates&>(this->ioa->getFinalStates());
		for (const auto& i: I3)
		{
			this->elsFSM.addFinalState((dynamic_cast<ELSState&>(*(i.second))));
		}
		return SMPLS::convertToMaxPlusAutomaton();
	}

	/*
		* after we generate the matrices through 'prepareMatrices', we have to make them
		* all square (to the size of the biggest matrix) by adding rows and cols of -infty
		*/
	void SMPLSwithEvents::makeMatricesSquare()
	{
		for (const auto& it: this->sm) {
			std::shared_ptr<Matrix> m = it.second;
			m->addCols(biggestMatrixSize - m->getCols());
			m->addRows(biggestMatrixSize - m->getRows());
		}
	}

	// transposes all matrices of the SMPLS
	void SMPLS::transposeMatrices()
	{
		for (auto& it: this->sm)
		{
			it.second = it.second->getTransposedCopy();
		}
	}
	/*
     * goes through the gamma relation and finds the event of the outcome
	 */
	Event SMPLSwithEvents::findEventByOutcome(const EventOutcome& outcome) const
	{
		for (const auto& gammaIterator: this->gamma)
		{
			// finding the event in gamma
			if ((EventOutcomePair(gammaIterator)).second == outcome)
			{
				return (EventOutcomePair(gammaIterator)).first;
			}
		}
		throw CException("Event of outcome " + outcome + " not found!");
	}

	/*
     * goes through the sigma relation and finds the event emitted by mode
	 */
	Event SMPLSwithEvents::findEventByMode(const Mode& mode) const
	{
		//find the event name ...
		for (const auto& sigmaIterator: this->sigma)
		{
			if ((ModeEventPair(sigmaIterator)).first == mode)
			{
				return (ModeEventPair(sigmaIterator)).second;
			}
		}
		throw CException("Event of mode " + mode + " not found!");
	}

	/**
	 * smpls with events needs to prepare the matrices to be able to perform analysis.
	 * this includes adding rows and columns of -inf and 0 based on the spec
	 * allowing the system to analyze processing or conveying event timings
	 */
	void SMPLSwithEvents::prepareMatrices(const IOAState& s, std::multiset<Event>& eventList, IOASetOfEdgeRefs& visitedEdges)
	{
		const auto& adj = dynamic_cast<const IOASetOfEdgeRefs&>(s.getOutgoingEdges());
		for (const auto *i: adj)
		{
			const auto *e = dynamic_cast<IOAEdgeRef>(i);
			if (visitedEdges.count(e) > 0) {
				continue;
			}

			//create a unique label for the new edge. this name will also be the mode name for sm
			CString modeName = CString(s.getId());
			modeName += ",";
			modeName += e->getLabel().first + "," + e->getLabel().second;

			// add the edge with unique name between the corresponding states
			this->elsFSM.addEdge(*this->elsFSM.getStateLabeled(s.getLabel()), modeName, *this->elsFSM.getStateLabeled((dynamic_cast<IOAStateRef>(e->getDestination()))->getLabel()));

			// make a copy so that child node can not modify the parent nodes list of events
			// only adds and removes and passes it to it's children
			std::multiset<Event> eList(eventList);

			OutputAction output = e->getLabel().second;
			InputAction input = e->getLabel().first;

			auto disSm = std::make_shared<DissectedModeMatrix>();
			std::shared_ptr<Matrix> sMatrix; //matrix to be generated and assigned to this edge
			CString outputActionEventName = "";
			int emittingEventIndex = -1;
			int processingEventIndex = -1;
			int eventIndexCounter = 0;
			if (!output.empty())
			{
				// find the dissected matrix structure needed for this edge
				disSm = findDissectedModeMatrix(output);
				if (disSm == nullptr){
					throw CException("mode " + output + " not found in dissected matrices!");
				}

				sMatrix = disSm->core.begin()->second->createCopy();

				// look if it has any events to emit
				if (!disSm->eventRows.empty())
				{
					outputActionEventName = findEventByMode(output);

					// ... and where it would sit in the sorted event std::multiset
					std::multiset<Event> eTempList(eList);
					eTempList.insert(outputActionEventName); // we insert in a copy to see where it would sit
					emittingEventIndex = static_cast<int>(eTempList.size() - 1);
					auto it = eTempList.rbegin();
					for (; it != eTempList.rend(); it++)
					{
						if (*it == outputActionEventName)
						{
							break;
						}
						emittingEventIndex--;
					}
				}
			}
			else
			{
				sMatrix = std::make_shared<Matrix>(numberOfResources, numberOfResources, MatrixFill::Identity);
			}
			if (!input.empty()) // find the index of the event to be removed from the list
			{
				Event e = findEventByOutcome(input);
				//we have to find the first occurrence from top
				processingEventIndex = static_cast<int>(distance(eList.begin(), eList.find(e)));
			}

			if (! eList.empty())
			{
				CString eventToBeRemoved;
				std::multiset<Event> eTempList(eList);
				std::multiset<Event>::iterator eventIter;
				for (eventIter = eTempList.begin(); eventIter != eTempList.end(); eventIter++)
				{
					sMatrix->addCols(1);

					// this means we have an event in the list which is being conveyed to the next matrix
					// so we add the necessary rows and cols to facilitate that

					if (eventIndexCounter == processingEventIndex) //we add B_c and B_e
					{
						uint cols = sMatrix->getCols();
						uint y = 0;
						for (; y < numberOfResources; y++) // B_c (refer to paper)
						{
							sMatrix->put(y, cols - 1, sMatrix->getMaxOfRow(y));
						}
						for (; y < numberOfResources; y++) // B_e (refer to paper)
						{
							sMatrix->put(y, cols - 1, getMaxOfRowUntilCol(*sMatrix, y, numberOfResources));
						}

						eList.erase(eList.lower_bound(*eventIter)); // event is processed, remove from eList
					}
					else // or are we conveying this event to the next state vector? I_c(q) (refer to paper)
					{
						sMatrix->addRows(1);											 // we already added a col of -infty, now we add the row
						sMatrix->put(sMatrix->getRows() - 1, sMatrix->getCols() - 1, MPTime(0)); // and 0 in the intersection of that row and col
					}
					// if we need to add the event row at the bottom
					if ((eventIndexCounter == emittingEventIndex) ||
						(eventIndexCounter + 1 == static_cast<int>(eTempList.size()) && emittingEventIndex == eventIndexCounter + 1)) // the second condition happens when we have reached the end of event list but we still need to add the event at the bottom
					{
						sMatrix->addRows(1);
						std::shared_ptr<Matrix> eventRow = *disSm->eventRows.begin(); // for now we just assume max one event emitted per mode
						uint cols = eventRow->getCols();
						uint x = 0;
						for (; x < cols; x++) // A_e (refer to paper)
						{
							sMatrix->put(sMatrix->getRows() - 1, x, eventRow->get(0, x));
						}
						for (; x < sMatrix->getCols(); x++) // B_e (refer to paper), this happens when we add B_c before adding A_e, so we have to make sure B_e that's added here is max of row
						{
							if (getMaxOfColUntilRow(*sMatrix, x, numberOfResources) > MP_MINUS_INFINITY) {
								// we must only add B_e under cols that correspond to B_c
								sMatrix->put(sMatrix->getRows() - 1, x, getMaxOfRowUntilCol(*eventRow, 0, numberOfResources));
							}
						}
					}

					eventIndexCounter++;
				}
			}
			else if (!output.empty()) {
				if (!disSm->eventRows.empty())
				{
					sMatrix->addRows(1);
					std::shared_ptr<Matrix> eventRow = *disSm->eventRows.begin();
					uint cols = eventRow->getCols(); // for now we just assume max one event emitted per mode

					for (uint x = 0; x < cols; x++)
					{
						sMatrix->put(sMatrix->getRows() - 1, x, eventRow->get(0, x));
					}
				}
}
			// we store the events emitted by modes as we move along the automaton
			if (!outputActionEventName.empty())
			{
				eList.insert(outputActionEventName);
			}

			this->sm[modeName] = sMatrix;
			visitedEdges.insert(e);

			// we need this later to make all matrices square
			biggestMatrixSize = std::max(biggestMatrixSize, std::max(sMatrix->getCols(), sMatrix->getRows()));

			prepareMatrices(*dynamic_cast<IOAStateRef>(e->getDestination()), eList, visitedEdges);
		}
	}

	std::shared_ptr<DissectedModeMatrix> SMPLSwithEvents::findDissectedModeMatrix(const CString& sName)
	{
		std::shared_ptr<DissectedModeMatrix> dis = nullptr;
		for (const auto& i: this->disMatrices) {
			if (i->core.begin()->first == sName)
			{
				dis = i;
				break;
			}
		}
		return dis;
	}

	/**
		 * recursive part of isConsistent
		 */
	void SMPLSwithEvents::isConsistentUtil(const IOAState& s, EventList& eventList, const IOASetOfStates& finalStates, CString& errMsg, std::map<IOAStateRef, EventList> &visited)
	{
		auto it = visited.find(&s);
		if (it != visited.end()) // we have already visited this state but we must check for inconsistency before leaving the state
		{
			if (!compareEventLists(it->second, eventList))
			{
				errMsg = CString("Different paths leading to different events at state " + std::to_string(s.getLabel()));
			}
			// we have checked for inconsistency, no need to go further down this state
			return;
		}
		visited.emplace(&s, eventList);

		if (finalStates.count(s.getLabel())>0)
		{
			if (!eventList.empty())
			{
				errMsg = "Event " + (*eventList.begin()) + " has not been processed by the end of the word.\r\n";
				return;
			}
		}
		else // If current state is not final
		{
			const auto& adj = dynamic_cast<const IOASetOfEdgeRefs&>(s.getOutgoingEdges());
			for (const auto *i: adj)
			{
				// make a copy so that child node can not modify the parent nodes list
				// only adds and removes and passes it to its children
				EventList eList(eventList);

				const auto *e = dynamic_cast<IOAEdgeRef>(i);
				const OutputAction output = e->getLabel().second;
				const InputAction input = e->getLabel().first;
				if (!input.empty()) {
					bool eventFound = false;
					EventList::iterator eventIter;
					for (eventIter = eList.begin(); eventIter != eList.end(); eventIter++)
					{
						for (const auto& gammaIterator: this->gamma)
						{
							auto p = (EventOutcomePair(gammaIterator));
							if (p.first == *eventIter) // finding the event in gamma
							{
								if (p.second == input)
								{
									eList.erase(eventIter); // event is processed, remove from eventList
									eventFound = true;
									break;
								}
							}
						}
						if (eventFound) {
							break;
						}
					}
					if (!eventFound)
					{
						// we are processing a non-emitted event
						errMsg = CString("on edge: " + std::to_string(s.getLabel()) + " - " + std::string(input + "," + output + ", Processing event outcome : " + input + " where the event is not emitted yet.\r\n"));
						return;
					}
				}
				// we store the events emitted by modes as we move along the automaton
				if (!output.empty())
				{
					// look if it has any events to emit
					std::list<ModeEventPair>::iterator sigmaIterator;
					for (const auto& sigmaIterator: this->sigma)
					{
						if ((ModeEventPair(sigmaIterator)).first == output) // this output action emits an event
						{
							auto a = (ModeEventPair(sigmaIterator));
							eList.push_back(a.second);
						}
					}
				}
				const auto s2 = dynamic_cast<IOAStateRef>(e->getDestination());

				isConsistentUtil(*s2, eList, finalStates, errMsg, visited);
			}
		}
	}

	/**
		 * recursive part of determinize
		 */
	void SMPLSwithEvents::determinizeUtil(const IOAState& s, IOASetOfStateRefs& visited, const IOASetOfStateRefs& finalStates, CString& errMsg, std::ofstream& outfile)
	{
		/**
		 * Deterministic IOA is defined with:
		 *	exactly one initial state;
		 *  final states that have no outgoing transition;
		 *	and branches only based on different outcomes of the same event;
		 */
		 // implementation is incomplete yet. we have to remove unreachable states and their edges after this
		if (visited.count(&s)>0) {
			return;
		}

		visited.insert(&s);

		const auto& outEdge = dynamic_cast<const IOASetOfEdgeRefs&>(s.getOutgoingEdges());
		auto i = outEdge.begin();

		const auto* e = dynamic_cast<const IOAEdge*>(*i);
		InputAction input = e->getLabel().first;
		if (input.empty())
		{
			const auto s2 = dynamic_cast<IOAStateRef>(e->getDestination());
			outfile << s.stateLabel << "-," << e->getLabel().second << "->" << s2->stateLabel;
			ioa->removeEdge(*e);

			i++;
			//we go through all the rest of the edges and remove them from the automaton
			for (; i != outEdge.end(); ++i)
			{
				e = dynamic_cast<IOAEdgeRef>(*i);
				ioa->removeEdge(*e);
				// since we only start with one initial state and remove the others, then the destination state of this edge will be unreachable
				//ioa->removeState(dynamic_cast<IOAState*>(e->getDestination()));
			}

			if (finalStates.count(s2)>0)
			{
				outfile << " f\n";
			}
			else {
				outfile << "\n";
				determinizeUtil(*s2, visited, finalStates, errMsg, outfile);
			}
		}
		else // we have an input action
		{
			Event ev = this->findEventByOutcome(input);
			//we go through all the edges
			for (; i != outEdge.end(); i++)
			{
				e = dynamic_cast<const IOAEdge*>(*i);
				input = e->getLabel().first;
				// we remove the ones that dont have an input action from the automaton
				if (input.empty())
				{
					ioa->removeEdge(*e);
					// since we only start with one initial state and remove the others, then the destination state of this edge will be unreachable
					//ioa->removeState(dynamic_cast<IOAState*>(e->getDestination()));
				}
				else
				{
					// only allow edges with the outcome of the same event
					if (this->findEventByOutcome(input) == ev)
					{
						const auto s2 = dynamic_cast<IOAStateRef>(e->getDestination());
						outfile << s.stateLabel << "-" << e->getLabel().first << "," << e->getLabel().second << "->" << s2->stateLabel;

						ioa->removeEdge(*e);
						if (finalStates.count(s2)>0)
						{
							outfile << " f\n";
						}
						else {
							outfile << "\n";
							determinizeUtil(*s2, visited, finalStates, errMsg, outfile);
						}
					}
					else
					{
						ioa->removeEdge(*e);
					}

				}
			}

		}

	}
	bool SMPLSwithEvents::compareEventLists(EventList& l1, EventList& l2)
	{
		bool result = true;

		if (l1.size() != l2.size())
		{
			return false;
		}

		auto it2 = l1.begin();
		for (; it2 != l1.end(); it2++)
		{
			result = false;
			auto it3 = l2.begin();
			for (; it3 != l2.end(); it3++)
			{
				if (*it2 == *it3)
				{
					result = true;
					break;
				}
			}
		}
		return result;
	}

	void SMPLSwithEvents::dissectModeMatrices()
	{
		ModeMatrices::iterator s;
		for (const auto& s: this->sm)
		{
			numberOfResources = 0;
			std::shared_ptr<DissectedModeMatrix> dis = std::make_shared<DissectedModeMatrix>();
			dis->core.insert(s);

			std::shared_ptr<Matrix> m = s.second; //->getTransposedCopy();
			std::list<uint> mSubIndices;
			numberOfResources = std::min(m->getRows(), m->getCols());
			for (uint x = 0; x < numberOfResources; x++)
			{
				mSubIndices.push_back(x);
				//numberOfResources++;
			}

			dis->core.begin()->second = m->getSubMatrixNonSquareRowsPtr(mSubIndices);
			if (m->getRows() > numberOfResources) // if we have event rows
			{
				mSubIndices.clear();
				for (uint x = numberOfResources; x < std::max(m->getRows(), m->getCols()); x++)
				{
					mSubIndices.push_back(x);
					std::shared_ptr<Matrix> mEventRow = m->getSubMatrixNonSquareRowsPtr(mSubIndices);
					dis->eventRows.push_back(mEventRow);
					mSubIndices.clear();
				}
			}
			disMatrices.push_back(dis);
		}
	}

}  // namespace MaxPlus::SMPLS
