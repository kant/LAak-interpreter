#include "Vector.h"

void Vector::register_methods(lua_State* L, luaL_Reg const* methods){
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);
    for(int i = 0; (methods+i)->name != nullptr; ++i){
        lua_pushcfunction(L, (methods+i)->func);
        lua_setfield(L, -2, (methods+i)->name);
    }
}

void Vector::register_vector(lua_State* L){
    luaL_newmetatable(L, vec_metatablename);

    register_methods(L, vector_methods);

    luaL_setfuncs(L, vector_functions, 0);
    lua_setglobal(L, vec_metatablename);
}

bool Vector::isvector(lua_State* L, int idx){
    lua_getmetatable(L, idx);
    lua_pushstring(L, "__name");
    lua_rawget(L, idx+1);
    return (strcmp(luaL_checkstring(L, -1), vec_metatablename) == 0);
}

VectorXd** Vector::check_vector(lua_State* L, int idx){
    void* ud = luaL_checkudata(L, idx, vec_metatablename);
    luaL_argcheck(L, ud != NULL, idx, "vector expected");
    return (VectorXd**)ud;
}

double* Vector::get_element(lua_State* L){
    VectorXd** v = check_vector(L);
    int index = luaL_checkinteger(L, 2);
    luaL_argcheck(L, 0 <= index && index < (*(*v)).size(), 2, "index out of range");
    return &(*(*v))[index];
}

int Vector::get_vecelem(lua_State* L){
    lua_pushnumber(L, *get_element(L));
    return 1;
}

int Vector::set_vecelem(lua_State* L){
    double newval = luaL_checknumber(L, 3);
    *get_element(L) = newval;
    return 0;
}

int Vector::new_vector(lua_State* L){
    int n = lua_gettop(L);
    if(n >= 1){
        VectorXd** v = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
        if(n == 1){
            //create vector with argument as size
            if(lua_isnumber(L,1)){
                int size = luaL_checkinteger(L, 1);
                *v = new VectorXd(size);
            //create vector with contents of table
            }else if(lua_istable(L,1)){
                *v = new VectorXd(lua_rawlen(L, 1));
                lua_pushnil(L);
                int idx = 0;
                while (lua_next(L, 1) != 0) {
                    (*(*v))[idx] = luaL_checknumber(L, -1);
                    idx++;
                    lua_pop(L, 1);
                }
            }else{
                return luaL_error(L, "Expected number or table/list");
            }
        }
        else{
            //create vector with multiple arguments
            *v = new VectorXd(n);
            for(int i = 1; i <= n; i++){
                (*(*v))[i-1] = luaL_checknumber(L, i);
            }
        }
    }else{
        return luaL_error(L, "Expected more arguments");
    }
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::free_vector(lua_State* L){
    VectorXd** v = check_vector(L);
    std::cout << *v << '\t' << "vector freed" << '\n';
    delete *v;
    return 0;
}

int Vector::add_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** b = check_vector(L, 2);

    if((*(*a)).size() != (*(*b)).size())
        return luaL_error(L, "Vector sizes are not the same");

    VectorXd** v = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *v = new VectorXd((*(*a)) + (*(*b)));

    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);

    return 1;
}

int Vector::sub_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** b = check_vector(L, 2);

    if((*(*a)).size() != (*(*b)).size())
            return luaL_error(L, "Vector sizes are not the same");

    VectorXd** v = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));

    *v = new VectorXd((*(*a)) - (*(*b)));
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::mul_vector(lua_State* L){
    double a;
    VectorXd** v;
    if(lua_isnumber(L,1)){
        a = luaL_checknumber(L, 1);
        v = check_vector(L, 2);
    }else if(lua_isnumber(L, 2)){
        a = luaL_checknumber(L, 2);
        v = check_vector(L);
    }else{
        v = Vector::check_vector(L, 1);
        MatrixXd** m = check_matrix(L, 2);
        if((*(*v)).cols() == (*(*m)).rows()){
            VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
            *r = new VectorXd((*(*v)) * (*(*m)));
            luaL_getmetatable(L, Vector::vec_metatablename);
            lua_setmetatable(L, -2);
            return 1;
        }else{
            return luaL_error(L, "Matrix columns and Vector rows are not the same");
        }
    }
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd(a * (*(*v)));
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::div_vector(lua_State* L){
    double a;
    VectorXd** v;
    if(lua_isnumber(L,1)){
        a = luaL_checknumber(L, 1);
        v = check_vector(L, 2);
    }else{
        a = luaL_checknumber(L, 2);
        v = check_vector(L);
    }
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd((1/a) * (*(*v)));
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::norm_vector(lua_State* L){
    VectorXd** v = check_vector(L);
    (*(*v)).normalize();
    return 0;
}

int Vector::normc_vector(lua_State* L){
    VectorXd** v = check_vector(L);
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd((*(*v)).normalized());
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 0;
}

int Vector::mag_vector(lua_State* L){
    VectorXd** v = check_vector(L);
    lua_pushnumber(L, (*(*v)).norm());
    return 1;
}

int Vector::dot_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** b = check_vector(L, 2);
    if((*(*a)).size() != (*(*b)).size())
        return luaL_error(L, "Vector sizes are not the same");
    auto d = (*(*a)).dot(*(*b));
    lua_pushnumber(L, d);
    return 1;
}

int Vector::cross_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** b = check_vector(L,2);
    if((*(*a)).size() != (*(*b)).size() || (*(*a)).size() != 3)
        return luaL_error(L, "Vector sizes are not the same and they should be 3 dimensions");
    Eigen::Vector<double, 3> aa, bb;
    aa = (*(*a));
    bb = (*(*b));
    auto v = aa.cross(bb);
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd(v);
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::T_vector(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd((*(*a)).size());
    (*(*r)) = (*(*a)).transpose();
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::type_vector(lua_State* L){
    VectorXd** a = check_vector(L);
    if((*(*a)).cols() == 1){
        lua_pushstring(L, "column vector");
    }else{
        lua_pushstring(L, "row vector");
    }
    return 1;
}

int Vector::get_vecsize(lua_State* L){
    VectorXd** v = check_vector(L);
    lua_pushinteger(L, (*(*v)).size());
    return 1;
}

int Vector::eq_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** b = check_vector(L, 2);
    lua_pushboolean(L, (*(*a)) == (*(*b)));
    return 1;
}

int Vector::unm_vectors(lua_State* L){
    VectorXd** a = check_vector(L);
    VectorXd** r = (VectorXd**)lua_newuserdata(L, sizeof(VectorXd*));
    *r = new VectorXd((*(*a)));
    (*(*r)) = -(*(*r));
    luaL_getmetatable(L, vec_metatablename);
    lua_setmetatable(L, -2);
    return 1;
}

int Vector::vector_tostring(lua_State* L){
    VectorXd** v = check_vector(L);
    std::ostringstream vosb;
    vosb << (*(*v));
    std::string vstr = vosb.str();
    // std::replace(vstr.begin(), vstr.end(), '\n', ',');
    // lua_pushfstring(L, "{ %s }", vstr.c_str());
    lua_pushfstring(L, "%s", vstr.c_str());
    return 1;
}