#include "symbolic/ControlFlowGraph.hpp"

using namespace symbolic;

/*** Helpers ***/

std::string Node::tostr() const {
	switch(type) {
		case MAIN: return "Main(" + std::to_string(id) + ")";
		case BLOCK: return "Block(" + std::to_string(id) + ")";
		case CALL: return "Call(" + std::to_string(id) + ")";
		case RETURN: return "Return(" + std::to_string(id) + ")";
		case ENTRY: return "Entry(" + std::to_string(id) + ")";
		case EXIT: return "Exit(" + std::to_string(id) + ")";
	}
}


/*** ControlFlowGraph::privates ***/

bool ControlFlowGraph::checkTransitionConstraints(StateType src, StateType dst) {
	switch(src) {
		case MAIN: switch (dst) {
			case MAIN: assert(true); return true;
			case BLOCK: assert(false); return false;
			case CALL: assert(true); return true;
			case RETURN: assert(false); return false;
			case ENTRY: assert(false); return false;
			case EXIT: assert(false); return false;
		}
		case BLOCK: switch (dst) {
			case MAIN: assert(false); return false;
			case BLOCK: assert(true); return true;
			case CALL: assert(true); return true;
			case RETURN: assert(true); return true;
			case ENTRY: assert(false); return false;
			case EXIT: assert(true); return true;
		}
		case CALL: switch (dst) {
			case MAIN: assert(false); return false;
			case BLOCK: assert(false); return false;
			case CALL: assert(false); return false;
			case RETURN: assert(false); return false; // Note: this was 'true' once
			case ENTRY: assert(false); return false; // Note: this was 'true' once
			case EXIT: assert(false); return false;
		}
		case RETURN: switch (dst) {
			case MAIN: assert(true); return true;
			case BLOCK: assert(true); return true;
			case CALL: assert(true); return true;
			case RETURN: assert(true); return true;
			case ENTRY: assert(false); return false;
			case EXIT: assert(true); return true;
		}
		case ENTRY: switch (dst) {
			case MAIN: assert(false); return false;
			case BLOCK: assert(true); return true;
			case CALL: assert(true); return true;
			case RETURN: assert(true); return true;
			case ENTRY: assert(false); return false;
			case EXIT: assert(true); return true;
		}
		case EXIT: switch (dst) {
			case MAIN: assert(false); return false;
			case BLOCK: assert(false); return false;
			case CALL: assert(false); return false;
			case RETURN: assert(false); return false;
			case ENTRY: assert(false); return false;
			case EXIT: assert(false); return false;
		}
	}
}

std::vector<BDD> ControlFlowGraph::init_vars(const Cudd& mgr, size_t numVars) {
	std::vector<BDD> vec;
	for (std::size_t i = 0; i < numVars; i++)
		vec.push_back(mgr.bddVar(i));
	return vec;
}

std::vector<BDD> ControlFlowGraph::init_varSubset(std::vector<BDD> vars, std::size_t begin, std::size_t range) {
	return std::vector<BDD>(vars.begin() + begin, vars.begin() + begin + range);
}

std::size_t ControlFlowGraph::offsetOf(StateType type) const {
	switch (type) {
		case MAIN: return 0;
		case BLOCK: return _numMainBlocks;
		case CALL: return offsetOf(BLOCK) + _numBlocks;
		case RETURN: return offsetOf(CALL) + _numCalls;
		case ENTRY: return offsetOf(RETURN) + _numCalls;
		case EXIT:	return offsetOf(ENTRY) + _numProcedures;
	}
}

std::size_t ControlFlowGraph::indexOf(Node node) const {
	return offsetOf(node.type) + node.id;
}

BDD ControlFlowGraph::encode(Node node, bool primed) const {
	assert(0 <= node.id);
	assert(imply(node.is(MAIN), node.id < _numMainBlocks));
	assert(imply(node.is(BLOCK), node.id < _numBlocks));
	assert(imply(node.is(CALL) || node.is(RETURN), node.id < _numCalls));
	assert(imply(node.is(ENTRY) || node.is(EXIT), node.id < _numProcedures));

	BDD state = primed ? _stateProtoPrimed : _stateProto;
	std::size_t index = indexOf(node);
	
	for (std::size_t pos = primed ? _offsetPrime : 0; index; pos++) {
		if (index & 1) state = state.Compose(!_vars[pos], pos);
		index >>= 1;
	}

	return state;
}


/*** ControlFlowGraph::publics ***/

ControlFlowGraph::ControlFlowGraph(std::size_t numMainBlocks, std::size_t numBlocks, std::size_t numProcedures, std::size_t numCalls, std::size_t numGlobalVariables, std::size_t numLocalVariables) :
	_numMainBlocks(numMainBlocks),
	_numBlocks(numBlocks),
	_numProcedures(numProcedures),
	_numCalls(numCalls),
	_numNodeVariables(ceil(log2(numMainBlocks + numBlocks + 2*numProcedures + 2*numCalls))),
	_numGlobVars(numGlobalVariables),
	_numTempVars(numLocalVariables),
	_numPVars(_numGlobVars + _numTempVars),
	_offsetPrime(_numNodeVariables + _numPVars),
	_offsetRel(2*_offsetPrime),
	_mgr(Cudd(2*_numNodeVariables + 2*_numPVars + _numGlobVars, 0)),
	_trans(_mgr.bddZero()),
	_calls(_mgr.bddZero()),
	_exits(_mgr.bddZero()),
	_returns(_mgr.bddZero()),
	_vars(init_vars(_mgr, 2*_numNodeVariables + 2*_numPVars + _numGlobVars)),
	_stateVars(init_varSubset(_vars, 0, _numNodeVariables)),
	_stateVarsPrimed(init_varSubset(_vars, _offsetPrime, _numNodeVariables)),
	_programVars(init_varSubset(_vars, _numNodeVariables, _numPVars)),
	_programVarsPrimed(init_varSubset(_vars, _offsetPrime + _numNodeVariables, _numPVars)),
	_globalVars(init_varSubset(_programVars, 0, _numGlobVars)),
	_globalVarsPrimed(init_varSubset(_programVarsPrimed, 0, _numGlobVars)),
	_globalVarsRel(init_varSubset(_vars, _offsetRel, _numGlobVars)),
	_localVars(init_varSubset(_programVars, _numGlobVars, _numTempVars)),
	_localVarsPrimed(init_varSubset(_programVarsPrimed, _numGlobVars, _numTempVars))
{
	// init prototypes for state encoding
	_stateProto = one();
	for (const auto &v : _stateVars) _stateProto *= !v;

	_stateProtoPrimed = one();
	for (const auto &v : _stateVarsPrimed) _stateProtoPrimed *= !v;

	// init return relation: CALL -> RETURN
	for (std::size_t i = 0; i < _numCalls; i++) {
		Call ith = Call(i);
		_returns += encode(ith.call()) * encode(ith.retrn(), true);
	}

	assert(_stateVars.size() == _numNodeVariables);
	assert(_stateVarsPrimed.size() == _numNodeVariables);
	assert(_programVars.size() == _numPVars);
	assert(_programVarsPrimed.size() == _numPVars);
	assert(_globalVars.size() == _numGlobVars);
	assert(_globalVarsPrimed.size() == _numGlobVars);
	assert(_globalVarsRel.size() == _numGlobVars);
	assert(_localVars.size() == _numTempVars);
	assert(_localVarsPrimed.size() == _numTempVars);

	/*output*///std::cout << "ControlFlowGraph created; _numGlobVars:" << _numGlobVars << "; _numTempVars: " << _numTempVars << std::endl;
}

void ControlFlowGraph::addTransition(Node src, Node dst, BDD guardedaction) {
	assert(checkTransitionConstraints(src.type, dst.type));
	// TODO: guardedaction must not contain state variables of any kind and doubly primed variables
	/*output*///std::cout << "-- adding transition: " << src.tostr() << " -> " << dst.tostr() << std::endl;

	_trans += encode(src) * encode(dst, true) * guardedaction;
}

void ControlFlowGraph::addCall(Call call, Procedure proc) {
	assert((_calls * encode(call.call())).IsZero());
	/*output*///std::cout << "-- adding call: " << call.call().tostr() << " -> " << proc.entry().tostr() << std::endl;

	_calls += encode(call.call()) * encode(proc.entry(), true);
	_exits += encode(proc.exit()) * encode(call.call(), true);
}

symbolic::Node ControlFlowGraph::decode(BDD state) const {
	// compute index from BDD
	std::size_t index = 0;
	for (auto it = _stateVars.rbegin(); it != _stateVars.rend(); it++) {
		if ((*it & state) != zero()) index++;
		index <<= 1;
	}
	index >>= 1;

	// find node with index -> type+id
	if (index < offsetOf(BLOCK)) return symbolic::Node(MAIN, index - offsetOf(MAIN));
	if (index < offsetOf(CALL)) return symbolic::Node(BLOCK, index - offsetOf(BLOCK));
	if (index < offsetOf(RETURN)) return symbolic::Node(CALL, index - offsetOf(CALL));
	if (index < offsetOf(ENTRY)) return symbolic::Node(RETURN, index - offsetOf(RETURN));
	if (index < offsetOf(EXIT)) return symbolic::Node(ENTRY, index - offsetOf(ENTRY));
	if (index < offsetOf(EXIT) + _numProcedures) return symbolic::Node(EXIT, index - offsetOf(EXIT));
	assert(false);
}
