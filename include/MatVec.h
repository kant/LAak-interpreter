#ifndef LUA
#define LUA
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}
#endif

#ifndef ALGORITHM
#define ALGORITHM
#include <algorithm>
#endif

#ifndef IOSTREAM
#define IOSTREAM
#include <iostream>
#endif

#ifndef EIGEN
#define EIGEN
#include "Eigen/Dense"
#include "Eigen/Geometry"
typedef Eigen::Vector<double, Eigen::Dynamic> VectorXd;
typedef Eigen::RowVector<double, Eigen::Dynamic> RowVectorXd;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixXd;
#endif

#ifndef MATVEC
#define MATVEC
// namespace for the shared functions between Matrix and Vector
namespace MatVec {
    inline static const char* mat_metatablename = "matrix";
    inline static const char* vec_metatablename = "vector";
    inline static const char* rowvec_metatablename = "rowvector";

    static MatrixXd** check_matrix(lua_State* L, int idx = 1){
        void* ud = luaL_checkudata(L, idx, mat_metatablename);
        luaL_argcheck(L, ud != NULL, idx, "matrix expected");
        return (MatrixXd**)ud;
    }

    static bool ismatrix(lua_State* L, int idx = 1){
        //painfull way to learn this shit, watch out from luaL_checkudata, does some crazy shit
        lua_getmetatable(L, idx);
        lua_pushstring(L, "__name");
        lua_rawget(L, idx+1);
        return (strcmp(luaL_checkstring(L, -1), mat_metatablename) == 0);
    }

    static bool alloc_matrix(lua_State* L, MatrixXd mv){
        MatrixXd** m = (MatrixXd**)lua_newuserdata(L, sizeof(MatrixXd*));
        *m = new MatrixXd(mv);
        luaL_getmetatable(L, mat_metatablename);
        lua_setmetatable(L, -2);
        return true;
    }

    static void** check_vector(lua_State* L, const char* name = vec_metatablename, int idx = 1){
        void* ud = luaL_checkudata(L, idx, name);
        luaL_argcheck(L, ud != NULL, idx, "vector expected");
        return (void**)ud;
    }

    static bool isvector(lua_State* L, const char* name = vec_metatablename, int idx = 1){
        lua_getmetatable(L, idx);
        lua_pushstring(L, "__name");
        lua_rawget(L, idx+1);
        return (strcmp(luaL_checkstring(L, -1), name) == 0);
    }

    static int alloc_vector(lua_State* L, void* vv, const char* name = vec_metatablename){
        if(strcmp(name, vec_metatablename) == 0){
            VectorXd** v = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
            *v = new VectorXd(*((VectorXd*)vv));
            luaL_getmetatable(L, name);
            lua_setmetatable(L, -2);
        }else{
            RowVectorXd** v = (RowVectorXd**)lua_newuserdata(L, sizeof(RowVectorXd*));
            *v = new RowVectorXd(*((RowVectorXd*)vv));
            luaL_getmetatable(L, name);
            lua_setmetatable(L, -2);
        }
        return 1;
    }
}
#endif