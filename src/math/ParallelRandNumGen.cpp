/*
 * Copyright (c) 2005 The University of Notre Dame. All Rights Reserved.
 *
 * The University of Notre Dame grants you ("Licensee") a
 * non-exclusive, royalty free, license to use, modify and
 * redistribute this software in source and binary code form, provided
 * that the following conditions are met:
 *
 * 1. Acknowledgement of the program authors must be made in any
 *    publication of scientific results based in part on use of the
 *    program.  An acceptable form of acknowledgement is citation of
 *    the article in which the program was described (Matthew
 *    A. Meineke, Charles F. Vardeman II, Teng Lin, Christopher
 *    J. Fennell and J. Daniel Gezelter, "OOPSE: An Object-Oriented
 *    Parallel Simulation Engine for Molecular Dynamics,"
 *    J. Comput. Chem. 26, pp. 252-271 (2005))
 *
 * 2. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * This software is provided "AS IS," without a warranty of any
 * kind. All express or implied conditions, representations and
 * warranties, including any implied warranty of merchantability,
 * fitness for a particular purpose or non-infringement, are hereby
 * excluded.  The University of Notre Dame and its licensors shall not
 * be liable for any damages suffered by licensee as a result of
 * using, modifying or distributing the software or its
 * derivatives. In no event will the University of Notre Dame or its
 * licensors be liable for any lost revenue, profit or data, or for
 * direct, indirect, special, consequential, incidental or punitive
 * damages, however caused and regardless of the theory of liability,
 * arising out of the use of or inability to use software, even if the
 * University of Notre Dame has been advised of the possibility of
 * such damages.
 */

#include "math/ParallelRandNumGen.hpp"
#ifdef IS_MPI
#include <mpi.h>
#endif

namespace oopse {

  int ParallelRandNumGen::nCreatedRNG_ = 0;

  ParallelRandNumGen::ParallelRandNumGen(const uint32& oneSeed) {

    const int masterNode = 0;
    int seed = oneSeed;
#ifdef IS_MPI
    MPI_Bcast(&seed, 1, MPI_UNSIGNED_LONG, masterNode, MPI_COMM_WORLD); 
#endif
    if (seed != oneSeed) {
      sprintf(painCave.errMsg,
	      "Using different seed to initialize ParallelRandNumGen.\n");
      painCave.isFatal = 1;;
      simError();
    }

    int nProcessors;
#ifdef IS_MPI
    MPI_Comm_size(MPI_COMM_WORLD, &nProcessors);
    MPI_Comm_rank( MPI_COMM_WORLD, &myRank_);
#else
    nProcessors = 1;
    myRank_ = 0;
#endif
    //In order to generate independent random number stream, the
    //actual seed used by random number generator is the seed passed
    //to the constructor plus the number of random number generators
    //which are already created.
    int newSeed = oneSeed + nCreatedRNG_;
    mtRand_ = new MTRand(newSeed, nProcessors, myRank_);
    
    ++nCreatedRNG_;
  }

  ParallelRandNumGen::ParallelRandNumGen() {

    std::vector<uint32> bigSeed;
    const int masterNode = 0;
    int nProcessors;
#ifdef IS_MPI
    MPI_Comm_size(MPI_COMM_WORLD, &nProcessors);
    MPI_Comm_rank( MPI_COMM_WORLD, &myRank_);
#else
    nProcessors = 1;
    myRank_ = 0;
#endif
    mtRand_ = new MTRand(nProcessors, myRank_);

    seed();       /** @todo calling virtual function in constructor is
                      not a good design */
  }


  void ParallelRandNumGen::seed( const uint32 oneSeed ) {

    const int masterNode = 0;
    int seed = oneSeed;
#ifdef IS_MPI
    MPI_Bcast(&seed, 1, MPI_UNSIGNED_LONG, masterNode, MPI_COMM_WORLD); 
#endif
    if (seed != oneSeed) {
      sprintf(painCave.errMsg,
	      "Using different seed to initialize ParallelRandNumGen.\n");
      painCave.isFatal = 1;;
      simError();
    }
    
    int newSeed = oneSeed +nCreatedRNG_;
    mtRand_->seed(newSeed);
    
    ++nCreatedRNG_;
  }
        
  void ParallelRandNumGen::seed() {

    std::vector<uint32> bigSeed;
    int size;
    const int masterNode = 0;
#ifdef IS_MPI
    if (worldRank == masterNode) {
#endif

      bigSeed = mtRand_->generateSeeds();
      size = bigSeed.size();

#ifdef IS_MPI
      MPI_Bcast(&size, 1, MPI_INT, masterNode, MPI_COMM_WORLD);        
      MPI_Bcast(&bigSeed[0], size, MPI_UNSIGNED_LONG, masterNode, MPI_COMM_WORLD); 
    }else {
      MPI_Bcast(&size, 1, MPI_INT, masterNode, MPI_COMM_WORLD);        
      bigSeed.resize(size);
      MPI_Bcast(&bigSeed[0], size, MPI_UNSIGNED_LONG, masterNode, MPI_COMM_WORLD); 
    }
#endif
    
    if (bigSeed.size() == 1) {
      mtRand_->seed(bigSeed[0]);
    } else {
      mtRand_->seed(&bigSeed[0], bigSeed.size());
    }

    ++nCreatedRNG_;
  }        
}
