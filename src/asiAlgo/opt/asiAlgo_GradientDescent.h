//-----------------------------------------------------------------------------
// Created on: 24 April 2017
// Created by: Quaoar
//-----------------------------------------------------------------------------
// Web: http://dev.opencascade.org/, http://quaoar.su/blog
//-----------------------------------------------------------------------------

#ifndef asiAlgo_GradientDescent_h
#define asiAlgo_GradientDescent_h

// asiAlgo includes
#include <asiAlgo_ArmijoRule.h>

// STD includes
#include <vector>

#define COUT_DEBUG
#if defined COUT_DEBUG
  #pragma message("===== warning: COUT_DEBUG is enabled")
#endif

//-----------------------------------------------------------------------------
// Optimization tool
//-----------------------------------------------------------------------------

//! Gradient descent optimization.
template <typename T_DIMENSION>
class asiAlgo_GradientDescent
{
public:

  typedef T_DIMENSION t_coord;

public:

  //! Search parameters.
  struct t_search_params
  {
    t_coord                                start;          //!< Starting point.
    int                                    num_iterations; //!< Max number of iterations.
    double                                 precision;      //!< Precision stopping criterion.
    double                                 step;           //!< Step size.
    bool                                   is_adaptive;    //!< Whether to use adaptive step.
    asiAlgo_FunctionWithGradient<t_coord>* pFunc;          //!< Target function.

    t_search_params() : num_iterations (0),
                        precision      (0.0),
                        step           (0.0),
                        is_adaptive    (false),
                        pFunc          (NULL)
    {}
  };

public:

  asiAlgo_GradientDescent(const t_search_params& params) : m_params(params) {}

public:

  //! \return solution.
  const t_coord& Solution() const
  {
    return m_argmin;
  }

  //! \return sampling points.
  const std::vector<t_coord>& Samples() const
  {
    return m_samples;
  }

public:

  bool Perform(int& num_iters)
  {
    num_iters     = 0;
    t_coord pos   = m_params.start;
    bool doStop   = false;
    bool isSolved = false;

    // Prepare Armijo rule
    asiAlgo_ArmijoRule<t_coord> Armijo;
    asiAlgo_ArmijoRule<t_coord>::t_search_params Armijo_params;
    Armijo_params.max_alpha      = m_params.step;
    Armijo_params.num_iterations = 100;
    Armijo_params.pFunc          = m_params.pFunc;

    // Main iterations
    do
    {
      ++num_iters;
      if ( num_iters < m_params.num_iterations )
      {
        m_samples.push_back(pos);

        t_coord g = m_params.pFunc->Gradient(pos);
        doStop = ( g.Modulus() < m_params.precision );

        if ( !doStop )
        {
          double actual_step = 0.0;

          // Use Armijo rule
          if ( m_params.is_adaptive )
          {
            Armijo_params.x_k = pos;
            Armijo_params.d_k = g.Invert(); // Anti-gradient is the search direction
            int num_armijo_iters = 0;

            if ( !Armijo.Perform(Armijo_params, num_armijo_iters, actual_step) )
            {
              // Convergence impossible. No sense to proceed, let's stop
              m_argmin = pos;
              doStop   = true;
            }
          }
          else
            actual_step = m_params.step;

          // Update rule
          pos = pos - g*actual_step;

          ///
          pos.Dump(std::cout);
          ///
#if defined COUT_DEBUG
          std::cout << "F[" << num_iters << ", step " << actual_step
                    << "] = " << m_params.pFunc->Value(pos) << std::endl;
#endif
        }
        else
        {
          m_argmin = pos;
          isSolved = true;
        }
      }
      else
      {
        m_argmin = pos;
        doStop   = true;
      }
    }
    while ( !doStop );

    return isSolved;
  }

private:

  std::vector<t_coord> m_samples; //!< Sampling points.
  t_search_params      m_params;  //!< Search parameters.
  t_coord              m_argmin;  //!< Solution.

};

#endif
