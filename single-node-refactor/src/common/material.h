/**********************************************************************************************
� 2020. Triad National Security, LLC. All rights reserved.
This program was produced under U.S. Government contract 89233218CNA000001 for Los Alamos
National Laboratory (LANL), which is operated by Triad National Security, LLC for the U.S.
Department of Energy/National Nuclear Security Administration. All rights in the program are
reserved by Triad National Security, LLC, and the U.S. Department of Energy/National Nuclear
Security Administration. The Government is granted for itself and others acting on its behalf a
nonexclusive, paid-up, irrevocable worldwide license in this material to reproduce, prepare
derivative works, distribute copies to the public, perform publicly and display publicly, and
to permit others to do so.
This program is open source under the BSD-3 License.
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1.  Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.
3.  Neither the name of the copyright holder nor the names of its contributors may be used
to endorse or promote products derived from this software without specific prior
written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

#ifndef FIERRO_MATERIAL_H
#define FIERRO_MATERIAL_H

#include <stdio.h>

#include "matar.h"

// eos files
#include "gamma_law_eos.h"
#include "no_eos.h"
#include "user_defined_eos.h"
#include "void_eos.h"

// strength
#include "no_strength.h"
#include "user_defined_strength.h"

// erosion files
#include "erosion.h"

// fracture files
#include "user_defined_fracture.h"


namespace model
{
    // strength model types
    enum strength_type
    {
        no_strength_type,
        increment_based,        ///<  Model evaluation is inline with the time integration
        state_based,            ///<  Model is based on the state after each stage of the time step
    };

    // Specific strength models
    enum strength_models
    {
        no_strength_model,
        user_defined_strength,
    };

    // EOS model types
    enum EOSType
    {
        noEOSType,          ///< No EOS used
        decoupledEOSType,   ///< only an EOS, or an EOS plus deviatoric stress model
        coupledEOSType,     ///< EOS is part of a full stress tensor evolution model
    };

    // The names of the eos models
    enum EOSModels
    {
        noEOS,              ///<  no model evaluation
        gammaLawGasEOS,     ///<  gamma law gas
        voidEOS,            ///<  a void material, no sound speed and no pressure
        userDefinedEOS,     ///<  an eos function defined by the user
    };

    // failure model types
    enum failure_type
    {
        no_failure,
        brittle_failure,        ///< Material fails after exceeding yield stress
        ductile_failure,        ///< Material grows voids that lead to complete failure
    };

    // erosion model types
    enum erosion_type
    {
        no_erosion,
        erosion,            ///<  element erosion
        erosion_contact,    ///<  element erosion and apply contact enforcement
    };
} // end model namespace

static std::map<std::string, model::strength_type> strength_type_map
{
    { "no_strength", model::no_strength_type },
    { "increment_based", model::increment_based },
    { "state_based", model::state_based },
};

static std::map<std::string, model::strength_models> strength_models_map
{
    { "no_strength", model::no_strength_model },
    { "user_defined_strength", model::user_defined_strength },
};

static std::map<std::string, model::EOSType> eos_type_map
{
    { "no_eos",    model::noEOSType },
    { "coupled",   model::coupledEOSType },
    { "decoupled", model::decoupledEOSType },
};

static std::map<std::string, model::EOSModels> eos_models_map
{
    { "no_eos",        model::noEOS },
    { "gamma_law_gas", model::gammaLawGasEOS },
    { "void",          model::voidEOS },
    { "user_defined",  model::userDefinedEOS },
};

static std::map<std::string, model::erosion_type> erosion_type_map
{
    { "no_erosion", model::no_erosion },
    { "erosion", model::erosion },
    { "erosion_contact", model::erosion_contact },
};

namespace model_init
{
// strength model setup
enum strength_setup_tag
{
    input = 0,
    user_init = 1,
};
} // end of namespace

/////////////////////////////////////////////////////////////////////////////
///
/// \struct material_t
///
/// \brief  Material models
///
/// In the code: CArrayKokkos <Material_t> Material;
/////////////////////////////////////////////////////////////////////////////
struct material_t
{
    size_t id;

    // -- EOS --
    // none, decoupled, or coupled eos
    model::EOSType EOSType = model::noEOSType;

    // Equation of state (EOS) function pointers
    void (*calc_pressure)(const DCArrayKokkos<double>& elem_pres,
                          const DCArrayKokkos<double>& elem_stress,
                          const size_t elem_gid,
                          const size_t mat_id,
                          const DCArrayKokkos<double>& elem_state_vars,
                          const DCArrayKokkos<double>& elem_sspd,
                          const double den,
                          const double sie) = NULL;

    void (*calc_sound_speed)(const DCArrayKokkos<double>& elem_pres,
                             const DCArrayKokkos<double>& elem_stress,
                             const size_t elem_gid,
                             const size_t mat_id,
                             const DCArrayKokkos<double>& elem_state_vars,
                             const DCArrayKokkos<double>& elem_sspd,
                             const double den,
                             const double sie) = NULL;

    // Strength model type: none, or increment- or state-based
    model::strength_type strength_type = model::no_strength_type;

    // Material strength model function pointers
    void (*calc_stress)(const DCArrayKokkos<double>& elem_pres,
                        const DCArrayKokkos<double>& elem_stress,
                        const size_t elem_gid,
                        const size_t mat_id,
                        const DCArrayKokkos<double>& elem_state_vars,
                        const DCArrayKokkos<double>& elem_sspd,
                        const double den,
                        const double sie,
                        const ViewCArrayKokkos<double>& vel_grad,
                        const ViewCArrayKokkos<size_t>& elem_node_gids,
                        const DCArrayKokkos<double>&    node_coords,
                        const DCArrayKokkos<double>&    node_vel,
                        const double vol,
                        const double dt,
                        const double rk_alpha) = NULL;

    // Material Failure: none or there is a failure model
    model::failure_type failure_type = model::no_failure;

    // -- Erosion --

    // erosion model
    model::erosion_type erosion_type = model::no_erosion;
    size_t void_mat_id;        ///< eroded elements get this mat_id
    double erode_tension_val;   ///< tension threshold to initiate erosion
    double erode_density_val;   ///< density threshold to initiate erosion
    // above should be removed, they go in CArrayKokkos<double> erosion_global_vars;

    // setup the strength model via the input file for via a user_setup
    model_init::strength_setup_tag strength_setup = model_init::input;

    size_t num_eos_state_vars = 0; ///< Number of state variables for the EOS
    size_t num_strength_state_vars  = 0; ///< Number of state variables for the strength model
    size_t num_eos_global_vars      = 0; ///< Number of global variables for the EOS
    size_t num_strength_global_vars = 0; ///< Number of global variables for the strength model

    DCArrayKokkos<double> eos_global_vars; ///< Array of global variables for the EOS

    DCArrayKokkos<double> strength_global_vars; ///< Array of global variables for the strength model

    double q1   = 1.0;      ///< acoustic coefficient in Riemann solver for compression
    double q1ex = 1.3333;   ///< acoustic coefficient in Riemann solver for expansion
    double q2   = 1.0;      ///< linear coefficient in Riemann solver for compression
    double q2ex = 1.3333;   ///< linear coefficient in Riemann solver for expansion

}; // end material_t

/////////////////////////////////////////////////////////////////////////////
///
/// \struct material_model_values_t
///
/// \brief  Material model state, parameters, and values 
///
/// In the code: CArray <MaterialModelValues_t> MaterialModelValues;
/////////////////////////////////////////////////////////////////////////////
struct MaterialModelValues_t{

    ///<enums can be implemented in the model namespaces to unpack e.g., physics_global_vars

    CArrayKokkos<double> eos_global_vars;      ///< Array of global variables for the EOS
    CArrayKokkos<double> eos_state_vars;       ///< Array of state (in each element) variables for the EOS

    CArrayKokkos<double> strength_global_vars; ///< Array of global variables for the strength model
    CArrayKokkos<double> strength_state_vars;  ///< Array of state (in each element) variables for the strength
    
    CArrayKokkos<double> failure_global_vars;  ///< Array of global variables for the failure model

    CArrayKokkos<double> erosion_global_vars;  ///< Array of global variables for the erosion model

    CArrayKokkos<double> art_viscosity_global_vars; ///< Array holding q1, q1ex, q2, ...

    // ...

}; // end MaterialModelValues_t
// The above struct eliminates all the variables in material_t, making material_t a collection of function ptrs


// ----------------------------------
// valid inputs for material options
// ----------------------------------
static std::vector<std::string> str_material_inps
{
    "id",
    "eos_model",
    "eos_model_type",
    "strength_model",
    "strength_model_type",
    "q1",
    "q2",
    "q1ex",
    "q2ex",
    "eos_global_vars",
    "erosion_type",
    "erode_tension_val",
    "erode_density_val",
    "void_mat_id",
};

// ----------------------------------
// required inputs for material options
// ----------------------------------
static std::vector<std::string> material_required_inps
{
    "id",
    "eos_model",
    "eos_model_type",
    // "eos_global_vars",
    // "strength_model",
    // "strength_model_type"
};




#endif // end Header Guard