#ifndef DUST_H
#define DUST_H

#include <cstdint>
#include <map>
#include <silo.h>
#include <Kokkos_Core.hpp>

class Dust {
  public:
    static const size_t NDUST=1024;
    enum STATE : uint32_t;
    using st_type = typename std::underlying_type<STATE>::type;

    typedef Kokkos::View<double   [NDUST]    > ScalarPointType;
    typedef Kokkos::View<double   [NDUST][3] > Vectr3PointType;
    typedef Kokkos::View<st_type  [NDUST]    > HealthPointType;
    typedef Kokkos::View<double   [NDUST][3] > LocatnPointType;
    typedef Kokkos::View<uint64_t [NDUST]    > SSNumbPointType;
    typedef LocatnPointType LocationVecType;
    typedef HealthPointType PointHealthType;

    ScalarPointType age  ;
    ScalarPointType dob  ;
    SSNumbPointType ssn  ;
    HealthPointType state;
    LocatnPointType loc  ;

    std::map<std::string, ScalarPointType> ScalarPointVariables;
    std::map<std::string, Vectr3PointType> Vectr3PointVariables;

    enum STATE : uint32_t {
       HEALTHY          =  0, //
       UNOCCUPIED           , //
       EXIT_LEFT_X_BDRY     , //
       EXIT_RGHT_X_BDRY     , //
       EXIT_LEFT_Y_BDRY     , //
       EXIT_RGHT_Y_BDRY     , //
       EXIT_LEFT_Z_BDRY     , //
       EXIT_RGHT_Z_BDRY     , //
       WENT_TOO_FAR         , // !Went farther than the nearest neighbor
       WENT_LEFT_X          , //
       WENT_RGHT_X          , //
       WENT_LEFT_Y          , //
       WENT_RGHT_Y          , //
       WENT_LEFT_Z          , //
       WENT_RGHT_Z          , //
       DEATH_HOLE           , // !Kill particles that defy logic
       NBR_NOSPACE          , // !Neighbor has no space to receive
       DIVISION_BY_ZERO     , //
       DRIFTED              , // !Drifted away from the intended location
       INVALID                //
    };

  public:
    // constructor
    Dust(uint64_t ssn_start=0) {
      age   = ScalarPointType ("age"  );
      dob   = ScalarPointType ("dob"  );
      ssn   = SSNumbPointType ("ssn"  );
      state = HealthPointType ("state");
      loc   = LocatnPointType ("loc"  );
      Kokkos::parallel_for(NDUST, init_ssn(ssn, ssn_start));
      Kokkos::fence();
    }
    /* ---------------------------------------------------------------------- */
    struct init_ssn {
      uint64_t ssn_start;
      Kokkos::View<uint64_t  [NDUST]    > ssn  ;
      //constructor
      init_ssn(Kokkos::View<uint64_t [NDUST]> ssn_, uint64_t ssn_start_)
        : ssn(ssn_), ssn_start(ssn_start_) { };
      KOKKOS_INLINE_FUNCTION
      void operator()(const size_t n) const {
        ssn(n) = ssn_start+n;
      }
    };
    /* ---------------------------------------------------------------------- */
    void write_silo(std::string prefix="Dust") const {
      char filename[100];
      sprintf(filename, "%s.silo", prefix.c_str());
      DBfile *file=nullptr;
      file=DBCreate(filename, DB_CLOBBER, DB_LOCAL, NULL, DB_HDF5);
      write_silo_mesh(file);
      DBClose(file);
      return;
    }
    /* ---------------------------------------------------------------------- */
    void write_silo_mesh(DBfile *file, std::string prefix="Points") const {
      std::vector<double> xvVec, yvVec, zvVec;
      get_vertex_coordinates(xvVec, yvVec, zvVec);
      double* coords[3]={xvVec.data(), yvVec.data(), zvVec.data()};
      char meshname[100];
      sprintf(meshname, "%s", prefix.c_str());
      DBPutPointmesh(file, meshname, 3, coords, NDUST, DB_DOUBLE, NULL);

      ScalarPointType::HostMirror   ageHost = Kokkos::create_mirror_view(  age);
      ScalarPointType::HostMirror   dobHost = Kokkos::create_mirror_view(  dob);
      SSNumbPointType::HostMirror   ssnHost = Kokkos::create_mirror_view(  ssn);
      HealthPointType::HostMirror stateHost = Kokkos::create_mirror_view(state);
      Kokkos::deep_copy(   ageHost,   age);
      Kokkos::deep_copy(   dobHost,   dob);
      Kokkos::deep_copy(   ssnHost,   ssn);
      Kokkos::deep_copy( stateHost, state);

      DBPutPointvar1 (file,   "age", "Points",   age.data(), NDUST, DB_DOUBLE, NULL);
      DBPutPointvar1 (file,   "dob", "Points",   dob.data(), NDUST, DB_DOUBLE, NULL);
      DBPutPointvar1 (file,   "ssn", "Points",   ssn.data(), NDUST, DB_LONG_LONG, NULL);
      DBPutPointvar1 (file, "state", "Points", state.data(), NDUST, DB_INT   , NULL);

      for(auto it : ScalarPointVariables) {
        ScalarPointType v=it.second;
        ScalarPointType::HostMirror vHost = Kokkos::create_mirror_view(v);
        Kokkos::deep_copy(vHost, v);
        DBPutPointvar1 (file, it.first.c_str(), "Points", vHost.data(), NDUST, DB_DOUBLE, NULL);
      }
      return;
    }
    /* ---------------------------------------------------------------------- */
    void get_vertex_coordinates(std::vector<double> &xvVec,
                                std::vector<double> &yvVec,
                                std::vector<double> &zvVec) const {
      xvVec.resize(NDUST);
      yvVec.resize(NDUST);
      zvVec.resize(NDUST);

      Vectr3PointType::HostMirror locHost =Kokkos::create_mirror_view(loc);
      Kokkos::deep_copy(locHost, loc);

      for(int n=0; n<NDUST; n++) {
        xvVec[n]=loc(n,0);
        yvVec[n]=loc(n,1);
        zvVec[n]=loc(n,2);
      }
      //RSA need to convert indices to coordinates
      return;
    }
    /* ---------------------------------------------------------------------- */

};

#endif
