/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2013, Willow Garage
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan, Luis G. Torres */

#ifndef OMPL_TEST_2DCIRCLES_SETUP_DUBINS_
#define OMPL_TEST_2DCIRCLES_SETUP_DUBINS_

#include <boost/filesystem.hpp>

#include "ompl/base/SpaceInformation.h"
#include "ompl/base/ScopedState.h"
#include "ompl/base/spaces/RealVectorStateSpace.h"
#include "ompl/base/spaces/SO2StateSpace.h"
#include "ompl/base/spaces/DubinsStateSpace.h"
#include "ompl/base/goals/GoalSampleableRegion.h"

#include "../../resources/config.h"
#include "../../resources/circles2D.h"

namespace ompl
{
    namespace geometric
    {

        class StateValidityChecker2DCirclesDubins : public base::StateValidityChecker
        {
        public:

            StateValidityChecker2DCirclesDubins(const base::SpaceInformationPtr &si, const Circles2D &circles) :
                base::StateValidityChecker(si),
                circles_(circles)
            {
            }

            virtual bool isValid(const base::State *state) const
            {
                const double *xy = state->as<base::DubinsStateSpace::StateType>()->
                    as<base::RealVectorStateSpace::StateType>(0)->values;
                return circles_.noOverlap(xy[0], xy[1]);
            }

        private:
            const Circles2D &circles_;
        };

        static base::SpaceInformationPtr spaceInformation2DCirclesDubins(const Circles2D &circles)
        {
            // TODO if stuff breaks maybe the turning radius is too
            // big for the circle environments
            base::DubinsStateSpace *space = new base::DubinsStateSpace();

            // Set bounds to match bounding box of input circles
            base::RealVectorBounds bounds(2);
            bounds.setLow(0, circles.minX_);
            bounds.setLow(1, circles.minY_);
            bounds.setHigh(0, circles.maxX_);
            bounds.setHigh(1, circles.maxY_);
            space->setBounds(bounds);

            base::SpaceInformationPtr si(new base::SpaceInformation(base::StateSpacePtr(space)));
            StateValidityChecker2DCirclesDubins *svc =
                new StateValidityChecker2DCirclesDubins(si, circles);
            si->setStateValidityChecker(base::StateValidityCheckerPtr(svc));
            si->setStateValidityCheckingResolution(0.002);
            si->setup();
            return si;
        }

        static base::RealVectorStateSpace* get2DSpaceFromDubins(base::StateSpacePtr dubinsSpace)
        {
            return dubinsSpace->as<base::DubinsStateSpace>()->as<base::RealVectorStateSpace>(0);
        }

        // Goal region that specifies 2D coordinate, but leaves
        // orientation free of constraint
        class DubinsXYGoal : public base::GoalSampleableRegion
        {
        public:
            DubinsXYGoal(const base::SpaceInformationPtr &si,
                         const base::State *state2D) :
                base::GoalSampleableRegion(si),
                goalState2D_(si)
            {
                const base::StateSpacePtr& so2 =
                    si->getStateSpace()->as<base::CompoundStateSpace>()->getSubspace(1);
                samplerSO2_ = si->getStateSpace()->allocSubspaceStateSampler(so2);

                si_->copyState(goalState2D_.get(), state2D);
            }

            virtual double distanceGoal(const base::State *dubinsState) const
            {
                return get2DSpaceFromDubins(si_->getStateSpace())->
                    distance(dubinsState->as<base::DubinsStateSpace::StateType>()->
                                 as<base::RealVectorStateSpace::StateType>(0),
                             goalState2D_.get()->as<base::DubinsStateSpace::StateType>()->
                                 as<base::RealVectorStateSpace::StateType>(0));
            }

            virtual void sampleGoal(base::State *state) const
            {
                si_->copyState(state, goalState2D_.get());
                samplerSO2_->sampleUniform(state);
            }

            virtual unsigned int maxSampleCount(void) const
            {
                return 100;
            }
        private:
            base::ScopedState<> goalState2D_;
            base::StateSamplerPtr samplerSO2_;
        };
    }
}

#endif
