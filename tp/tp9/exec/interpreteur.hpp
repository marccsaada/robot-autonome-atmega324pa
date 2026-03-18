#pragma once
#include "Robot.hpp"
#include "Memoire.hpp"


class Interpreteur { 
    public: 
    Interpreteur();
    void sequenceDemarrage();
    void executer();


    private: 
    Robot robot_;
    Memoire memoire_; 

}; 