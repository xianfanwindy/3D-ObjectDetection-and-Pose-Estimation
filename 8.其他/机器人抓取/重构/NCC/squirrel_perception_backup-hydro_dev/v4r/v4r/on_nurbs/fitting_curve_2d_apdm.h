/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2012-, Open Perception, Inc.
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
 *   * Neither the name of the copyright holder(s) nor the names of its
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
 *
 * 
 *
 */

#ifndef NURBS_FITTING_CURVE_2D_APDM_H
#define NURBS_FITTING_CURVE_2D_APDM_H

#include "nurbs_tools.h"
#include "nurbs_data.h"
#include "nurbs_solve.h"

namespace pcl
{
  namespace on_nurbs
  {
    /** \brief Fitting a 2D B-Spline curve to 2D point-clouds using asymmetric point-distance-minimization
      * Based on paper: TODO
      * \author Thomas Mörwald
      * \ingroup surface
      */
    class FittingCurve2dAPDM
    {
      public:

        /** \brief Parameters for fitting */
        struct Parameter
        {
          double interior_sigma2;
          double smoothness;
          double closest_point_weight;
          double closest_point_sigma2;
          unsigned closest_point_resolution;
          double smooth_concavity;
          double rScale;
          Parameter () :
            interior_sigma2 (0.1), smoothness (0.1), closest_point_weight (0.1), closest_point_sigma2 (0.1),
                closest_point_resolution (0), smooth_concavity (1.0), rScale (1.0)
          {
          }
        };

        struct FitParameter
        {
          pcl::on_nurbs::FittingCurve2dAPDM::Parameter param;
          double addCPsAccuracy;        // add CPs at points where this threshold is exceeded
          unsigned addCPsIteration;     // add CPs at every "addCPsIteration" iteration
          unsigned maxCPs;              // maximum number of CPs

          double accuracy;              // threshold for incremental change of CPs (termination criterion)
          unsigned iterations;          // maximum number of fitting iterations
        };

        ON_TextLog m_out;
        ON_NurbsCurve m_nurbs;
        NurbsDataCurve2d *m_data;

        /** \brief Constructor initializing B-Spline curve using initNurbsCurve2D(...).
         * \param[in] order the polynomial order of the B-Spline curve.
         * \param[in] data pointer to the 2D point-cloud data to be fit.        */
        FittingCurve2dAPDM (int order, NurbsDataCurve2d *data);

        /** \brief Constructor initializing with the B-Spline curve given in argument 2.
         * \param[in] data pointer to the 2D point-cloud data to be fit.
         * \param[in] nc B-Spline curve used for fitting.        */
        FittingCurve2dAPDM (NurbsDataCurve2d *data, const ON_NurbsCurve &nc);

        /** \brief Find the element in which the parameter xi lies.
         * \param[in] xi value in parameter domain of the B-Spline curve.
         * \param[in] elements the vector of elements of the curve.
         * \return index of the element with respect to elements. */
        static int
        findElement (double xi, const std::vector<double> &elements);

        /** \brief Refines curve by inserting a knot in the middle of each element. */
        void
        refine ();

        /** \brief Refines curve by inserting a knot in the middle of the element belonging to xi.
         *  \param[in] xi parameter defining the element to be refined. */
        void
        refine (double xi);

        /** \brief Fitting with iterative refinement and adaptive knot insertion
         *  \param[in] param The parameters for fitting*/
        void
        fitting (FitParameter &param);

        /** \brief Assemble the system of equations for fitting
         * - for large point-clouds this is time consuming.
         * - should be done once before refinement to initialize the starting points for point inversion. */
        virtual void
        assemble (const Parameter &parameter);

        /** \brief Solve system of equations using Eigen or UmfPack (can be defined in on_nurbs.cmake),
         *  and updates B-Spline curve if a solution can be obtained. */
        virtual double
        solve (double damp = 1.0);

        /** \brief Update curve according to the current system of equations.
         *  \param[in] damp damping factor from one iteration to the other. */
        virtual double
        updateCurve (double damp);

        /** \brief Adds control points in the middle and of elements,
         *  if the ends or the middle of the element is farther than max_error to the closest data point
         *  \param[in] max_error defining the max allowed distance between data points and curve. */
        void
        addCPsOnClosestPointViolation (double max_error);

        /** \brief Removes control points when it is collinear with its neighboring control points.
         *  \param[in] nurbs the B-Spline curve to be altered.
         *  \param[in] min_curve_th threshold for dot product of vector to neighboring control points (negative).
         *  \return the altered curve.*/
        static ON_NurbsCurve
        removeCPsOnLine (const ON_NurbsCurve &nurbs, double min_curve_th = -0.9);

        /** \brief Initialize a closed B-Spline curve given a list of control points.
         *  \param[in] order polynomial order of the curve.
         *  \param[in] cps sequence of control points.
         *  \return B-Spline curve. */
        static ON_NurbsCurve
        initCPsNurbsCurve2D (int order, const vector_vec2d &cps);

        /** \brief Initialize a closed B-Spline curve using the bounding circle of the point-cloud.
         *  \param[in] order polynomial order of the curve.
         *  \param[in] data 2D point-cloud
         *  \param[in] ncps number of control points
         *  \param[in] radiusF radius factor multiplied to the radius of the bounding circle.
         *  \return B-Spline curve. */
        static ON_NurbsCurve
        initNurbsCurve2D (int order, const vector_vec2d &data, int ncps = 0, double radiusF = 1.0);
        //  static ON_NurbsCurve initNurbsCurvePCA(int order, const vector_vec2d &data);

        /** \brief Get the elements of a B-Spline curve.*/
        static std::vector<double>
        getElementVector (const ON_NurbsCurve &nurbs);

        /** \brief Inverse mapping / point inversion: Given a point pt, this function finds the closest
         * point on the B-Spline curve using Newtons method and point-distance (L2-, Euclidean norm).
         *  \param[in] nurbs the B-Spline curve.
         *  \param[in] pt the point to which the closest point on the curve will be computed.
         *  \param[in] hint the starting point in parametric domain (warning: may lead to convergence at local minima).
         *  \param[in] error the distance between the point pt and p after convergence.
         *  \param[in] p closest point on curve.
         *  \param[in] t the tangent vector at point p.
         *  \param[in] maxSteps maximum number of iterations.
         *  \param[in] accuracy convergence criteria: if error is lower than accuracy the function returns
         *  \return closest point on curve in parametric domain.*/
        static double
        inverseMapping (const ON_NurbsCurve &nurbs, const Eigen::Vector2d &pt, const double &hint, double &error,
                        Eigen::Vector2d &p, Eigen::Vector2d &t, double rScale, int maxSteps = 100,
                        double accuracy = 1e-6, bool quiet = true);

        static double
        inverseMappingO2 (const ON_NurbsCurve &nurbs, const Eigen::Vector2d &pt, double &error, Eigen::Vector2d &p,
                          Eigen::Vector2d &t);

        /** \brief Given a point pt, the function finds the closest midpoint of the elements of the curve.
         *  \param[in] nurbs the B-Spline curve.
         *  \param[in] pt the point to which the closest midpoint of the elements will be computed.
         *  return closest midpoint in parametric domain. */
        static double
        findClosestElementMidPoint (const ON_NurbsCurve &nurbs, const Eigen::Vector2d &pt);
        static double
        findClosestElementMidPoint (const ON_NurbsCurve &nurbs, const Eigen::Vector2d &pt, double hint);

        /** \brief Enable/Disable debug outputs in console. */
        inline void
        setQuiet (bool val)
        {
          m_quiet = val;
          m_solver.setQuiet (val);
        }

        /** \brief Set parameters for inverse mapping. */
        inline void
        setInverseParams (int max_steps = 200, double accuracy = 1e-6)
        {
          in_max_steps = max_steps;
          in_accuracy = accuracy;
        }

      protected:
        /** \brief Add minimization constraint: point-to-curve distance (point-distance-minimization). */
        virtual void
        addPointConstraint (const double &param, const Eigen::Vector2d &point, double weight, unsigned &row);

        /** \brief Add minimization constraint: smoothness by control point regularisation. */
        virtual void
        addCageRegularisation (double weight, unsigned &row, const std::vector<double> &elements, double wConcav = 0.0);

        /** \brief Assemble point-to-curve constraints. */
        virtual void
        assembleInterior (double wInt, double sigma2, double rScale, unsigned &row);

        /** \brief Assemble closest points constraints. At each midpoint of the curve elements the closest data points
         * are computed and point-to-curve constraints are added. */
        virtual void
        assembleClosestPoints (const std::vector<double> &elements, double weight, double sigma2,
                               unsigned samples_per_element, unsigned &row);

        NurbsSolve m_solver;
        bool m_quiet;
        int in_max_steps;
        double in_accuracy;
    };
  }
}

#endif
