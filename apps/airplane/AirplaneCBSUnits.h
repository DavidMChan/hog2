//
//  AirplaneCBSUnits.h
//  hog2 glut
//
//  Created by David Chan on 6/8/16.
//  Copyright (c) 2016 University of Denver. All rights reserved.
//

#ifndef __hog2_glut__AirplaneCBSUnits__
#define __hog2_glut__AirplaneCBSUnits__

#include <iostream>
#include <limits> 
#include <algorithm>
#include <map>

#include <queue>
#include <functional>
#include <vector>

#include <thread>
#include <mutex>

#include "Unit.h"
#include "UnitGroup.h"
#include "Airplane.h"
#include "AirplaneConstrained.h"
//#include "AirplaneTicketAuthority.h"
#include "TemplateIntervalTree.h"
#include "TemplateAStar.h"
#include "BFS.h"
#include "Heuristic.h"
#include "Timer.h"

struct IntervalData{
  IntervalData(uint64_t h1, uint64_t h2, uint8_t a):hash1(h1),hash2(h2),agent(a){}
  uint64_t hash1;
  uint64_t hash2;
  uint8_t agent;
  bool operator==(IntervalData const& other)const{return other.hash1==hash1 && other.hash2==hash2 && other.agent==agent;}
};


extern bool highsort;
extern bool randomalg;
extern bool useCAT;
extern unsigned killtime;
typedef TemplateIntervalTree<IntervalData,float> IntervalTree;
typedef TemplateInterval<IntervalData,float> Interval;
extern IntervalTree* CAT; // Conflict Avoidance Table
extern AirplaneConstrainedEnvironment* currentEnv;
extern uint8_t currentAgent;

template <class state>
struct CompareLowGCost;
template <class state>
struct RandomTieBreaking;

extern TemplateAStar<airtimeState, airplaneAction, AirplaneConstrainedEnvironment, AStarOpenClosed<airtimeState, RandomTieBreaking<airtimeState> > >* currentAstar;

template <class state>
struct CompareLowGCost {
  bool operator()(const AStarOpenClosedData<state> &i1, const AStarOpenClosedData<state> &i2) const
  {
    if (fequal(i1.g+i1.h, i2.g+i2.h))
    {
      return fless(i1.data.t,i2.data.t); // Break ties by time
    }
    return (fgreater(i1.g+i1.h, i2.g+i2.h));
  }
};

// Check if an openlist node conflicts with a node from an existing path
unsigned checkForConflict(airtimeState const*const parent, airtimeState const*const node, airtimeState const*const pathParent, airtimeState const*const pathNode);

template <class state>
struct RandomTieBreaking {
  bool operator()(const AStarOpenClosedData<state> &ci1, const AStarOpenClosedData<state> &ci2) const
  {
    if (fequal(ci1.g+ci1.h, ci2.g+ci2.h)) // F-cost equal
    {
      if(useCAT && CAT && ci1.data.nc==0 && ci2.data.nc==0){
        // Make them non-const :)
        AStarOpenClosedData<state>& i1(const_cast<AStarOpenClosedData<state>&>(ci1));
        AStarOpenClosedData<state>& i2(const_cast<AStarOpenClosedData<state>&>(ci2));

        //std::cout << "ITSIZE " << CAT->size() << "\n";
        auto matches(CAT->findOverlapping(i1.data.t,i1.data.t+.1));

        // Get number of conflicts in the parent
        airtimeState const*const parent1(i1.parentID?&(currentAstar->GetItem(i1.parentID).data):nullptr);
        unsigned nc1(parent1?parent1->nc:0);
        //std::cout << "matches " << matches.size() << "\n";

        // Count number of conflicts
        for(auto const& m: matches){
          if(currentAgent == m.value.agent) continue;
          airtimeState p(currentEnv->GetState(m.value.hash1));
          p.t=m.start;
          airtimeState n(currentEnv->GetState(m.value.hash2));
          n.t=m.stop;
          nc1+=checkForConflict(parent1,&i1.data,&p,&n);
          //if(!nc1){std::cout << "NO ";}
          //std::cout << "conflict(1): " << i1.data << " " << n << "\n";
        }
        // Set the number of conflicts in the data object
        i1.data.nc=nc1;

        // Do the same for node 2
        matches = CAT->findOverlapping(i2.data.t,i2.data.t+.1);
        airtimeState const*const parent2(i2.parentID?&(currentAstar->GetItem(i2.parentID).data):nullptr);
        unsigned nc2(parent2?parent2->nc:0);

        // Count number of conflicts
        for(auto const& m: matches){
          if(currentAgent == m.value.agent) continue;
          airtimeState p(currentEnv->GetState(m.value.hash1));
          p.t=m.start;
          airtimeState n(currentEnv->GetState(m.value.hash2));
          n.t=m.stop;
          nc2+=checkForConflict(parent2,&i2.data,&p,&n);
          //if(!nc2){std::cout << "NO ";}
          //std::cout << "conflict(2): " << i2.data << " " << n << "\n";
        }
        // Set the number of conflicts in the data object
        i2.data.nc=nc2;
        //std::cout << "NC " << nc1 << " " << nc2 << " @ "<<i1.data <<" "<<i2.data<<"\n";
        //std::cout << "\n";

        return fless(i2.data.nc,i1.data.nc);
      }
      else if(randomalg && fequal(ci1.g,ci2.g)){
        return rand()%2;
      }
      else {
        return (fless(ci1.g, ci2.g));  // Tie-break toward greater g-cost
      }
    }
    return (fgreater(ci1.g+ci1.h, ci2.g+ci2.h));
  }
};


class AirCBSUnit : public Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> {
public:
	AirCBSUnit(std::vector<airtimeState> const &gs)
	:start(0), goal(1), current(gs[0]), waypoints(gs) {}
	const char *GetName() { return "AirCBSUnit"; }
	bool MakeMove(AirplaneConstrainedEnvironment *,
            OccupancyInterface<airtimeState,airplaneAction> *, 
            SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *,
            airplaneAction& a);
	void UpdateLocation(AirplaneConstrainedEnvironment *, airtimeState &newLoc, bool success, 
						SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *)
	{ if (success) current = newLoc; else assert(!"CBS Unit: Movement failed"); }
	
	void GetLocation(airtimeState &l) { l = current; }
	void OpenGLDraw(const AirplaneConstrainedEnvironment *, const SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *) const;
	void GetGoal(airtimeState &s) { s = waypoints[goal]; }
	void GetStart(airtimeState &s) { s = waypoints[start]; }
	inline std::vector<airtimeState> const & GetWaypoints()const{return waypoints;}
	inline airtimeState GetWaypoint(size_t i)const{ return waypoints[std::min(i,waypoints.size()-1)]; }
        inline unsigned GetNumWaypoints()const{return waypoints.size();}
	void SetPath(std::vector<airtimeState> &p);
	void PushFrontPath(std::vector<airtimeState> &s)
	{
		std::vector<airtimeState> newPath;
		for (airtimeState x : s)
			newPath.push_back(x);
		for (airtimeState y : myPath)
			newPath.push_back(y);
		myPath = newPath;
	}
	inline std::vector<airtimeState> const& GetPath()const{return myPath;}
    void UpdateGoal(airtimeState &start, airtimeState &goal);
        void setUnitNumber(unsigned n){number=n;}
        unsigned getUnitNumber()const{return number;}

private:
	unsigned start, goal;
        airtimeState current;
	std::vector<airtimeState> waypoints;
	std::vector<airtimeState> myPath;
        unsigned number;
};

struct airConflict {
	airConstraint c;
	int unit1;
        int prevWpt;
};

struct AirCBSTreeNode {
	AirCBSTreeNode():parent(0),satisfiable(true){}
	std::vector< std::vector<airtimeState> > paths;
	airConflict con;
	unsigned int parent;
	bool satisfiable;
        IntervalTree cat; // Conflict avoidance table
};

static std::ostream& operator <<(std::ostream & out, const AirCBSTreeNode &act)
{
	out << "(paths:"<<act.paths.size()<<", parent: "<<act.parent<< ", satisfiable: "<<act.satisfiable<<")";
	return out;
}

struct EnvironmentContainer {
	EnvironmentContainer() : name("NULL ENV"), environment(0), heuristic(0), conflict_cutoff(0), astar_weight(0.0f) {}
	EnvironmentContainer(std::string n, AirplaneConstrainedEnvironment* e, Heuristic<airtimeState>* h, uint32_t conf, float a) : name(n), environment(e), heuristic(h), conflict_cutoff(conf), astar_weight(a) {}
	AirplaneConstrainedEnvironment* environment;
	Heuristic<airtimeState>* heuristic;
	uint64_t conflict_cutoff;
	float astar_weight;
	std::string name;
};


class AirCBSGroup : public UnitGroup<airtimeState, airplaneAction, AirplaneConstrainedEnvironment>
{
public:
	AirCBSGroup(std::vector<EnvironmentContainer> const&, bool u_r, bool u_w, bool);
	bool MakeMove(Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> *u, AirplaneConstrainedEnvironment *e, 
				  SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *si, airplaneAction& a);
	void UpdateLocation(Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> *u, AirplaneConstrainedEnvironment *e, 
						airtimeState &loc, bool success, SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *si);
	void AddUnit(Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> *u);
	void UpdateUnitGoal(Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> *u, airtimeState newGoal);
	void UpdateSingleUnitPath(Unit<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> *u, airtimeState newGoal);
	
	void OpenGLDraw(const AirplaneConstrainedEnvironment *, const SimulationInfo<airtimeState,airplaneAction,AirplaneConstrainedEnvironment> *)  const;
	double getTime() {return time;}
	bool donePlanning() {return planFinished;}
	void ExpandOneCBSNode(bool gui=true);

	std::vector<AirCBSTreeNode> tree;
private:    

        unsigned IssueTicketsForNode(int location);
        unsigned LoadConstraintsForNode(int location);
        bool Bypass(int best, unsigned numConflicts, airConflict const& c1, bool gui);
	void Replan(int location);
        unsigned HasConflict(std::vector<airtimeState> const& a, std::vector<airtimeState> const& b, int x, int y, airConflict &c1, airConflict &c2, bool update, bool verbose=false);
	unsigned FindFirstConflict(AirCBSTreeNode const& location, airConflict &c1, airConflict &c2);
        void processSolution();

	void DoHAStar(airtimeState& start, airtimeState& goal, std::vector<airtimeState>& thePath);
	bool HAStarHelper(airtimeState& start, airtimeState& goal, std::vector<airtimeState>& thePath, unsigned& envConflicts, unsigned& conflicts);
	
	bool planFinished;

	/* Code for dealing with multiple environments */
	std::vector<EnvironmentContainer> environments;
	EnvironmentContainer* currentEnvironment;

	void SetEnvironment(unsigned);
    void ClearEnvironmentConstraints();
    void AddEnvironmentConstraint(airConstraint c);

	//std::vector<airtimeState> thePath;
	TemplateAStar<airtimeState, airplaneAction, AirplaneConstrainedEnvironment, AStarOpenClosed<airtimeState, RandomTieBreaking<airtimeState> > > astar;
	TemplateAStar<airtimeState, airplaneAction, AirplaneConstrainedEnvironment, AStarOpenClosed<airtimeState, CompareLowGCost<airtimeState> > > astar2;
	//TemplateAStar<airtimeState, airplaneAction, AirplaneConstrainedEnvironment> astar3;
	double time;

	unsigned int bestNode;
	std::mutex bestNodeLock;

	struct OpenListNode {
		OpenListNode() : location(0), cost(0), nc(0) {}
		OpenListNode(uint loc, double c, uint16_t n) : location(loc), cost(c),nc(n) {}
                std::ostream& operator <<(std::ostream& out)const{
                  out << "(loc: "<<location<<", nc: "<<nc<< ", cost: "<<cost<<")";
                  return out;
                }

		uint location;
		double cost;	
                unsigned nc;
	};
	struct OpenListNodeCompare {
          bool operator() (const OpenListNode& left, const OpenListNode& right) {
            if(highsort)
               return (left.nc==right.nc)?(left.cost > right.cost):(left.nc>right.nc);
             else
              return (left.cost==right.cost)?(left.nc > right.nc):(left.cost>right.cost);
          }
	};

	std::priority_queue<AirCBSGroup::OpenListNode, std::vector<AirCBSGroup::OpenListNode>, AirCBSGroup::OpenListNodeCompare> openList;

	uint TOTAL_EXPANSIONS = 0;
        Timer* timer=0;

	//TicketAuthority ticketAuthority;

	bool use_restricted = false;
	bool use_waiting = false;
	bool nobypass = false;
        std::vector<AirplaneConstrainedEnvironment*> agentEnvs;
};


#endif /* defined(__hog2_glut__AirplaneCBSUnits__) */
