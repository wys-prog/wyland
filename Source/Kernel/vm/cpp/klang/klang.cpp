// コクウキョウ

// Kokūkyō - Kokuyo for Wyland Project
// Modified version of Kokuyo specifically built for the Wyland project.
// This code is open-source and free to use, modify, and distribute.

// This file is a custom and simple programming language, designed for 
// Kokūkyō Virtual Machine.

#include <any>
#include <tuple>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include <unordered_map>

namespace kokuyo {
  namespace klang {

    typedef struct {
      int code; /* Type’s representation */
    } type;

    /* Define basic types */
    type typeof_dylib{256};
    type typeof_string{985}; /* These numbers are TOTALY random. Because it’s fun ! */
    type nil{-985}; /* It’s just funny random numbers. */

    class object {
    protected:
      void *data;
      type mytype;

      /* Because we’re using « polymorphisme », we cannot directly known members 
         of the given class. However, the `get_type()` method can give us the virtual type
         of the class. By knowing the type, we can simply call a member, wich is stored
         on an unordered map. The public call method asks you a string (for function’s name)
         and a vector (of arguments). */
      std::unordered_map<std::string, std::function<void(std::vector<std::any>)>> members;

    public:
      object &operator=(const object &obj) {
        data = obj.data;
        mytype = obj.mytype;

        return *this;
      }

      type get_type() const { return mytype;}
      void *get() const { return data; }
      void set(void *ptr, type t) {}

      void call(const std::string &name, const std::vector<std::any> &argv) {
        if (members.find(name) != members.end()) {
          members[name](argv); /* call the function, and give arguments */
        } else { /* member not found. Throwing an exception. */
          throw std::bad_function_call(); /* We cannot call NOTHING. */
        }
      }

    };

    class dylib : private object {
    private:
      /* Open a dynamic linked librarie (file in .so, .dll & .dylib) */
      void open(const std::string &path) { 
        
      }

    public:
      dylib &operator=(const dylib &dl) {
        this->set(dl.get(), typeof_dylib);
        return *this;
      }


    };

    typedef struct {
      std::vector<std::string> code;
      std::string ID;
      std::string name;
      std::unordered_map<std::string, object> scope;
    } task;

    class runtime {
    private:
      std::string env;
      std::vector<task> tasks;
    public:
    };

  } // namespace klang
  
} // namespace kokuyo
