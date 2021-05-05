#pragma once

#include "db_sta/dbReadVerilog.hh"
#include "opendb/db.h"
#include "sta/Liberty.hh"

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace ord {
    
    //******************************************************
    //**** Utility Function:  Convert number to string
    //*****************************************************
    std::string numberToString(double a);

    bool cmpPair(std::pair<sta::LibertyCell*, int>& a, std::pair<sta::LibertyCell*, int>& b);


    //******************************************************
    //************ Struct Connection
    //******************************************************
    struct Connection
    {
        sta::Instance *_driver = nullptr;
        sta::Instance *_load = nullptr;
        int _bundle_net = 0;
    };

    Connection* createConnection(sta::Instance* driver, sta::Instance* load);

    std::vector<Connection>::iterator connectionFind(std::vector<Connection>& connections, Connection& c);
   




    //******************************************************
    //************ Struct CommonConnection
    //******************************************************
    struct CommonConnection
    {
        sta::Instance *_load_a = nullptr;
        sta::Instance *_load_b = nullptr;
        int _bundle_net = 0;
    };

    CommonConnection* createCommonConnection(sta::Instance* load_a, sta::Instance* load_b);

    std::vector<CommonConnection>::iterator commonConnectionFind(std::vector<CommonConnection>& common_connections, CommonConnection& c);




    //******************************************************
    //************ Struct LumpedConnection
    //******************************************************
    struct LumpedConnection
    {
        int _fanout = 0;
        int _fanin  = 0;
        int _common_fanin = 0;
    };

    LumpedConnection* createLumpedConnection(int fanout, int fanin, int common_fanin = 0);


    
    
    
    //*****************************************************
    //***********  Class Module
    //*****************************************************
    class Module {
        private:
            dbVerilogNetwork *_network;

            sta::Instance *_inst;
            
            std::vector<sta::Instance*> _std_cells;
            std::vector<sta::Instance*> _submodules;
            std::vector<sta::Instance*> _macros;
            std::map<sta::LibertyCell*, int> _total_map_cells;
            std::vector<sta::Net*> _nets;  // the nets of within current module
            
            std::vector<Connection> _connections;    
            std::vector<CommonConnection> _common_connections;
            std::map<std::string, std::map<std::string, LumpedConnection> > _lumped_connections;


            int _total_std_cells = 0;
            int _total_comb_cells = 0;
            int _total_seq_cells = 0;
            int _total_buf_inv_cells = 0;
            int _total_macros = 0;
            int _total_comb_cell_pins = 0;
            int _total_comb_nets = 0;
            int _total_comb_fanouts = 0;
            int _num_pins = 0;
            double _total_std_cell_area = 0.0;  // unit: micron ^ 2
            double _total_macro_area = 0.0; // unit:  micron ^ 2
            double _total_area = 0.0;  // unit:  micron ^ 2
            double _std_cell_area = 0.0; // unit: micron ^ 2
            double _macro_area = 0.0;  // unit:  micron ^ 2
            double _total_comb_cell_area = 0.0;  // unit: micron ^ 2
            double _total_seq_cell_area = 0.0;   // unit: micron ^ 2
            double _total_buf_inv_cell_area = 0.0;  // unit: micron ^2
            
            bool specifyConnectionsFlag = false;
            bool detailed_flag = false; 
            // Utility function
            void printNetUtil(sta::Net *net, std::ofstream& file);
            
            void addConnectionUtil(sta::Net *net);
            void calculateConnectionsUtil();
            
            void calculateCommonConnectionsUtil();
            
            void calculateAverageCombFanoutsUtil(sta::Instance *inst);

            bool hasTerminals(sta::Net *net);
            

        public:
            Module();
            Module(dbVerilogNetwork *network, sta::Instance *inst) : _network(network) , _inst(inst) {  }
            
            void specifyDetailedFlag() { detailed_flag = true; }
            bool getDetailedFlag() { return detailed_flag; }
            // Accessor
            sta::Instance*  getInstance() { return _inst; }
            std::vector<sta::Instance*> getStdCells() { return _std_cells; }
            std::vector<sta::Instance*> getSubmodules() { return _submodules; }
            std::vector<sta::Instance*> getMacros() { return _macros; }
            std::map<sta::LibertyCell*, int> getTotalMapCells()  { return _total_map_cells; }

            std::string getVerilogName() {
                return std::string(_network->name(_inst));
            }

            std::string getPathName() {
                return std::string(_network->pathName(_inst));
            }

            std::string getName() {
                sta::Cell *cell = _network->cell(_inst);
                return std::string(_network->name(cell));
            }


            std::size_t getNumberStdCells() { return _std_cells.size(); }
            std::size_t getNumberSubmodules() { return _submodules.size(); }
            std::size_t getNumberMacros() { return _macros.size(); }
            int getNumberPins() { return _num_pins; }
            int getTotalStdCells() { return _total_std_cells; }
            int getTotalCombCells() { return _total_comb_cells; }
            int getTotalSeqCells() { return _total_seq_cells; }
            int getTotalBufInvCells() { return _total_buf_inv_cells; }
            int getTotalCombCellPins() { return _total_comb_cell_pins; }
            int getTotalMacros() { return _total_macros; }
            double getTotalArea() { return _total_area; }
            double getTotalStdCellArea() { return _total_std_cell_area; }
            double getTotalMacroArea() { return _total_macro_area; }
            double getTotalCombCellArea() { return _total_comb_cell_area; }
            double getTotalSeqCellArea() { return _total_seq_cell_area; }
            double getTotalBufInvCellArea()  { return _total_buf_inv_cell_area; }

            double getStdCellArea() { return _std_cell_area; }
            double getMacroArea() { return _macro_area; }
            
            int getTotalCombNets() { return _total_comb_nets; }
            double getAverageCombFanouts() { return _total_comb_fanouts * 1.0 / _total_comb_nets; }


            // Functions of adding elements
            void addStdCell(sta::Instance *inst) {
                _std_cells.push_back(inst);
            }

            
            void addTotalMapCell(sta::Instance* inst)
            {
                sta::Cell *cell = _network->cell(inst);
                sta::LibertyCell *liberty_cell = _network->libertyCell(cell);
                std::map<sta::LibertyCell*, int>::iterator map_it = _total_map_cells.find(liberty_cell);

                if(map_it != _total_map_cells.end()) {
                    map_it->second = map_it->second + 1;
                } else {
                    _total_map_cells.insert(std::pair<sta::LibertyCell*, int>(liberty_cell,1));
                }
            }
            
            void addSubmodule(sta::Instance *inst) {
                _submodules.push_back(inst);
            }

            void addMacro(sta::Instance *inst) {
                _macros.push_back(inst);
            }
            
            void addNet(sta::Net *net)
            {
                _nets.push_back(net);
            }
           
            void specifyNumberPins(int num_pins) {
                _num_pins = num_pins;
            }

            // Function of specifying values
            void specifyConnections()
            {
                if(specifyConnectionsFlag == false)
                {
                    specifyConnectionsFlag = true;
                    std::vector<sta::Net*>::iterator vec_net_it;
                    for(vec_net_it = _nets.begin(); vec_net_it != _nets.end(); vec_net_it++)
                    {   
                        addConnectionUtil(*vec_net_it);
                    }
                    calculateConnectionsUtil();
                    calculateCommonConnectionsUtil(); 
                }
            }

            
            void calculateAverageCombFanouts();

            void specifyTotalStdCells(int total_std_cells) { _total_std_cells = total_std_cells; }
            void specifyTotalMacros(int total_macros) { _total_macros = total_macros; }
            void specifyTotalCombCells(int total_comb_cells) { _total_comb_cells = total_comb_cells; }
            void specifyTotalSeqCells(int total_seq_cells) { _total_seq_cells = total_seq_cells; }
            void specifyTotalBufInvCells(int total_buf_inv_cells) { _total_buf_inv_cells = total_buf_inv_cells; }
            void specifyTotalCombCellPins(int total_comb_cell_pins) { _total_comb_cell_pins = total_comb_cell_pins; }

            void specifyTotalStdCellArea(double total_std_cell_area) { _total_std_cell_area = total_std_cell_area; }
            void specifyTotalMacroArea(double total_macro_area) { _total_macro_area = total_macro_area; }
            void specifyTotalArea(double total_area) { _total_area = total_area; }
            void specifyTotalCombCellArea(double total_comb_cell_area) { _total_comb_cell_area = total_comb_cell_area; }
            void specifyTotalSeqCellArea(double total_seq_cell_area) { _total_seq_cell_area = total_seq_cell_area; }
            void specifyTotalBufInvCellArea(double total_buf_inv_cell_area)
            { 
                _total_buf_inv_cell_area = total_buf_inv_cell_area;
            }

            void specifyStdCellArea(double std_cell_area) { _std_cell_area = std_cell_area; }
            void specifyMacroArea(double macro_area) { _macro_area = macro_area; }


            // Functions of printing
            void printAreaInfo(std::ofstream& file)
            {
                file << std::endl;
                file << std::setw(40) << " " << std::left << std::setw(20) << "Number" << std::setw(20) << "Area(um^2)" << std::endl;
                file << std::string(80, '-') << std::endl;
                file << std::setw(40) << std::left << "Standard Cells" << std::setw(20) << _total_std_cells;
                file << std::setw(20) << numberToString(_total_std_cell_area) << std::endl;
                file << std::setw(40) << "Combinational Standard Cells" << std::setw(20) << _total_comb_cells;
                file << std::setw(20) << numberToString(_total_comb_cell_area) << std::endl;
                file << std::setw(40) << "Inv/Buffer Cells" << std::setw(20) << _total_buf_inv_cells;
                file << std::setw(20) << numberToString(_total_buf_inv_cell_area) << std::endl;
                file << std::setw(40) << "Sequential Cells" << std::setw(20) << _total_seq_cells;
                file << std::setw(20) << numberToString(_total_seq_cell_area) << std::endl;
                file << std::setw(40) << "Hard Macros" << std::setw(20) << _total_macros;
                file << std::setw(20) << numberToString(_total_macro_area) << std::endl;
                file << std::string(80, '-') << std::endl;
                file << std::setw(60) << std::left << "Total Area";
                file << std::setw(20) << numberToString(_total_area) << std::endl;
            }


            void printNetInfo(std::ofstream& file)
            {
                file << "The total number of nets:  " << _nets.size() << std::endl;
                std::vector<sta::Net*>::iterator vec_net_it;
                for(vec_net_it = _nets.begin(); vec_net_it != _nets.end(); vec_net_it++)
                {
                    const char *net_name = _network->name(*vec_net_it);
                    file << "Net name:   " << net_name << std::endl;
                    printNetUtil(*vec_net_it, file);
                }
            }


            void printConnectionInfo(std::ofstream& file)
            {   
                file << "Connection Information" << std::endl;
                file << std::endl;
                
                std::map<std::string, std::map<std::string, LumpedConnection> >::iterator map_map_it;
                std::map<std::string, LumpedConnection>::iterator map_it;

                for(map_map_it = _lumped_connections.begin(); map_map_it != _lumped_connections.end(); map_map_it++)
                {
                    file << std::endl;
                    file << "Show the connection information of module:   " << map_map_it->first << std::endl;
                    for(map_it = map_map_it->second.begin(); map_it != map_map_it->second.end(); map_it++)
                    {
                        file << "\t\t\t" << "module:   " << std::setw(40) << std::left <<  map_it->first << "\t\t\t" << "number of nets:  " ;
                        file << map_it->second._fanout + map_it->second._fanin << " ( ";
                        file << map_it->second._fanout << " , ";
                        file << map_it->second._fanin  << " , ";
                        file << map_it->second._common_fanin << " ) " << std::endl;
                    }
                }
            }

            
            void printHierInfo(std::ofstream& file)
            {
                file << std::setw(15) << numberToString(_total_area) + " |";
                file << std::setw(25) << std::to_string(_total_std_cells) + "/" + numberToString(_total_std_cell_area) + " |";
                file << std::setw(25) << std::to_string(_total_macros) + "/" + numberToString(_total_macro_area) + " |";
                file << std::setw(20) << std::to_string(_std_cells.size()) + "/" + numberToString(_std_cell_area) + " |";
                file << std::setw(20) << std::to_string(_macros.size()) + "/" + numberToString(_macro_area) + " |";
                file << std::setw(15) << std::to_string(_submodules.size()) + " |";
                if(_inst == _network->topInstance()) {
                    file << std::setw(70) << std::left << std::string(getName()) + " || NULL" << std::right << std::endl;
                } else {
                    file << std::setw(70) << std::left << std::string(getPathName()) + std::string(" || ") + getName() << std::right << std::endl;
                }
            }

            void printLogicalInfo(std::ofstream& file, int level)
            {
                file << "\"module_name\" : \"" << getName() << "\"," <<  std::endl;
                file << "\"level\" : " << level << "," <<  std::endl;
                file << "\"pins\" : " << _num_pins << "," << std::endl;
                file << "\"total_insts\" : " << _total_std_cells << "," << std::endl;
                file << "\"total_macros\" : " << _total_macros << "," << std::endl;
                file << "\"total_area\" : " << numberToString(_total_macro_area + _total_std_cell_area) << "," << std::endl;
                file << "\"local_insts\" : " << _std_cells.size() << "," << std::endl;
                file << "\"local_macros\" : " << _macros.size() << "," << std::endl;
            }
    };


    //*****************************************************
    //***********  Struct Node
    //*****************************************************
    struct Node {
        Module* _module;
        Node *_parent;
        std::vector<Node*> _children;
    }; 

    // Utility function
    Node* createNode(Module *module);




    //*****************************************************
    //***********  Class designBrowserKernel
    //*****************************************************
    class designBrowserKernel
    {   
        protected:
            dbVerilogNetwork *_network = nullptr;
            odb::dbDatabase *_db = nullptr;
            Node *_root = nullptr;
            
            std::queue<sta::Instance*> _module_queue;
            std::vector<Module> _module_list;
            int _level;
            

            std::vector<Module>::iterator moduleFind(sta::Instance *inst);
            Node* findNode(Node *node, std::string name);

            void createTreeUtil(Node *node);
            int specifyTotalStdCellsUtil(Node *node);
            int specifyTotalMacrosUtil(Node *node);
            int specifyTotalCombSeqCellsUtil(Node *node);
            int specifyTotalBufInvCellsUtil(Node *node);
            int specifyTotalCombCellPinsUtil(Node *node);
        

            double specifyTotalAreaUtil(Node *node);
            double specifyTotalStdCellAreaUtil(Node *node);
            double specifyTotalMacroAreaUtil(Node *node);
            double specifyTotalCombSeqCellAreaUtil(Node *node);
            double specifyTotalBufInvCellAreaUtil(Node *node);
            

            void addTotalMapCellsUtil(Node *node, Module* module);
           
            //////////////////////////////////////////////////////////////////////////////
            // Note:  dbuToMeters, metersToDbu, area are copied from Resizer.cc file
            double dbuToMicrons(int dist) const;
 
            int micronsToDbu(double dist) const;
 
            double area(odb::dbMaster *master);
            //////////////////////////////////////////////////////////////////////////////                       
            void createTree();

            void linkDesignBrowserKernelUtil(sta::Instance *inst);

            void traverseDepthUtil(Node *node, int value, std::ofstream& file);

            void reportLogicAreaKeyTermCountUtil(Node* node, std::ofstream& file);
            
            void reportLogicAreaKeyCellCountUtil(Node* node, std::ofstream& file);
            
            void reportMacroUtil(Node *node, std::ofstream& file);
            
            void reportDesignFileUtil(Node *node, int level, std::ofstream& file);

        public:
            designBrowserKernel() {  }
            void init(dbVerilogNetwork *network, odb::dbDatabase *db)
            {
                _network = network;
                _db = db;
            }

            void specifyLevel(int level) { _level = level; }
        
            void linkDesignBrowserKernel (dbVerilogNetwork* network, odb::dbDatabase *db);
 
            void traverseDesign(std::string name, std::ofstream& file);
 
            void reportLogicArea(std::string name, std::ofstream& file, bool detailed, const char* key);
            
            void reportNet(std::string name, std::ofstream& file);

            void reportConnection(std::string name, std::ofstream& file);

            void reportMacro(std::string name, std::ofstream& file);
            
            void reportDesignFile(std::ofstream& file);

            // Accessor
            dbVerilogNetwork* getNetwork() { return _network; }

    }; 


    //***************************************************************************************************
    //******** Interface functions
    //***************************************************************************************************

    designBrowserKernel* makeDesignBrowser();

    void deleteDesignBrowser(designBrowserKernel* browser);

    void dbLinkDesignBrowser(designBrowserKernel* browser, dbVerilogNetwork *network, odb::dbDatabase *db);

    void dbDesignBrowser(designBrowserKernel* browser, const char* name, const char *file_name, int level = 3);

    void dbReportLogicArea(designBrowserKernel* browser, const char* name, const char *file_name, bool detailed, const char *key);

    void dbReportLogicNet(designBrowserKernel* browser, const char* name, const char *file_name);

    void dbReportLogicConnection(designBrowserKernel* browser, const char* name, const char *file_name);

    void dbReportMacro(designBrowserKernel* browser, const char* name, const char *file_name);
    
    void dbReportDesignFile(designBrowserKernel* browser, const char *file_name);

} // namespace ord
