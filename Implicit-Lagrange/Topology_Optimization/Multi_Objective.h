#ifndef MULTI_OBJECTIVE_TOPOPT_H
#define MULTI_OBJECTIVE_TOPOPT_H

#include "matar.h"
#include <string>
#include <vector>
#include <Teuchos_ScalarTraits.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_oblackholestream.hpp>
#include <Teuchos_Tuple.hpp>
#include <Teuchos_VerboseObject.hpp>

#include <Tpetra_Core.hpp>
#include <Tpetra_Map.hpp>
#include <Tpetra_MultiVector.hpp>
#include <Kokkos_View.hpp>
#include "Tpetra_Details_makeColMap.hpp"
#include "Tpetra_Details_DefaultTypes.hpp"

#include "ROL_Types.hpp"
#include <ROL_TpetraMultiVector.hpp>
#include "ROL_Objective.hpp"
#include "ROL_Elementwise_Reduce.hpp"

class MultiObjective_TopOpt : public ROL::Objective<real_t> {
  
  typedef Tpetra::Map<>::local_ordinal_type LO;
  typedef Tpetra::Map<>::global_ordinal_type GO;
  typedef Tpetra::Map<>::node_type Node;
  typedef Tpetra::Map<LO, GO, Node> Map;
  typedef Tpetra::MultiVector<real_t, LO, GO, Node> MV;
  typedef ROL::Vector<real_t> V;
  typedef ROL::TpetraMultiVector<real_t,LO,GO,Node> ROL_MV;
  
  using traits = Kokkos::ViewTraits<LO*, Kokkos::LayoutLeft, void, void>;
  using array_layout    = typename traits::array_layout;
  using execution_space = typename traits::execution_space;
  using device_type     = typename traits::device_type;
  using memory_traits   = typename traits::memory_traits;
  using global_size_t = Tpetra::global_size_t;

  typedef Kokkos::View<real_t*, Kokkos::LayoutRight, device_type, memory_traits> values_array;
  typedef Kokkos::View<GO*, array_layout, device_type, memory_traits> global_indices_array;
  typedef Kokkos::View<LO*, array_layout, device_type, memory_traits> indices_array;
  
  //typedef Kokkos::DualView<real_t**, Kokkos::LayoutLeft, device_type>::t_dev vec_array;
  typedef MV::dual_view_type::t_dev vec_array;
  typedef MV::dual_view_type::t_host host_vec_array;
  typedef Kokkos::View<const real_t**, array_layout, HostSpace, memory_traits> const_host_vec_array;
  typedef MV::dual_view_type dual_vec_array;

private:
  
  Teuchos::RCP<MV> multi_derivative;
  ROL::Ptr<ROL_MV> ROL_Multi_Derivative;
  std::vector<ROL::Ptr<ROL::Objective<real_t>>> Multi_Objective_Terms_;
  std::vector<real_t> Multi_Objective_Weights_;
  int nobjectives;
  bool derivative_allocated;

  ROL::Ptr<const MV> getVector( const V& x ) {
    return dynamic_cast<const ROL_MV&>(x).getVector();
  }

  ROL::Ptr<MV> getVector( V& x ) {
    return dynamic_cast<ROL_MV&>(x).getVector();
  }

public:

  MultiObjective_TopOpt(std::vector<ROL::Ptr<ROL::Objective<real_t>>> Multi_Objective_Terms, std::vector<real_t> Multi_Objective_Weights){  
    Multi_Objective_Terms_ = Multi_Objective_Terms;
    Multi_Objective_Weights_ = Multi_Objective_Weights;
    nobjectives = Multi_Objective_Terms_.size();
    derivative_allocated = false;
    //derivative storage
    multi_derivative = Teuchos::rcp(new MV(FEM_->map, 1));
  }

  void update(const ROL::Vector<real_t> &z, ROL::UpdateType type, int iter = -1 ) {
    for(int iobjective = 0; iobjective < nobjectives; iobjective++){
      Multi_Objective_Terms_[iobjective]->update(z, type, iter);
    }
  }

  real_t value(const ROL::Vector<real_t> &z, real_t &tol) {
    //std::cout << "Started obj value on task " <<FEM_->myrank  << std::endl;
    ROL::Ptr<const MV> zp = getVector(z);
    real_t c = 0.0;
    for(int iobjective = 0; iobjective < nobjectives; iobjective++){
      c += Multi_Objective_Weights_[iobjective]*Multi_Objective_Terms_[iobjective]->value(z, tol);
    }
    //std::cout << "Ended obj value on task " <<FEM_->myrank  << std::endl;
    return c;
  }

  //void gradient_1( ROL::Vector<real_t> &g, const ROL::Vector<real_t> &u, const ROL::Vector<real_t> &z, real_t &tol ) {
    //g.zero();
  //}
  
  void gradient( ROL::Vector<real_t> &g, const ROL::Vector<real_t> &z, real_t &tol ) {
    //get Tpetra multivector pointer from the ROL vector
    ROL::Ptr<const MV> zp = getVector(z);
    ROL::Ptr<MV> gp = getVector(g);

    if(!derivative_allocated){

    }
    gp->putScalar(0);

  }
  
  
  void hessVec( ROL::Vector<real_t> &hv, const ROL::Vector<real_t> &v, const ROL::Vector<real_t> &z, real_t &tol ) {
    // Unwrap hv
    ROL::Ptr<MV> hvp = getVector(hv);

    // Unwrap v
    ROL::Ptr<const MV> vp = getVector(v);
    ROL::Ptr<const MV> zp = getVector(z);
    
    host_vec_array objective_hessvec = hvp->getLocalView<HostSpace> (Tpetra::Access::ReadWrite);
    const_host_vec_array design_densities = zp->getLocalView<HostSpace> (Tpetra::Access::ReadOnly);
    const_host_vec_array direction_vector = vp->getLocalView<HostSpace> (Tpetra::Access::ReadOnly);
  }

};

#endif // end header guard
