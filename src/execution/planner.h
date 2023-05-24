#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <memory>
#include <algorithm>
#include <vector>
#include "ast.h"
#include "Lock_manager.h"

enum value_type{
    integer,
    character
};

class value{
public:
    value_type type;
    std::string col_name;
    std::string tab_name;
    std::string str;
    int len;
};

class record{
public:
    std::vector<std::shared_ptr<value>> row;

    void pt_row(){
        for(auto iter: row){
            std::cout << iter->str << " ";
        }
    }
};

class KV{
public:
    std::vector<std::shared_ptr<record>> records;
    
    std::vector<int> Key_range(){
        std::vector<int> res{0,1};
        return res;
    }
    void get(int key, std::shared_ptr<record>& res){
        res = records[key];
    }
};
//--------------以上暂时数据结构---------------//

class Operators{
public:
    std::shared_ptr<Operators> next_node;
    std::shared_ptr<KV> kv;           //此处后续转成context, 包含事务指针, 存储指针等等
    virtual std::vector<std::shared_ptr<record>> exec_op() = 0;
};

std::vector<std::shared_ptr<record>> send_plan(std::string node_name, std::shared_ptr<Operators> node_plan);

class op_excutor: public Operators{
public:
    std::vector<std::shared_ptr<record>> exec_op(){
        return next_node->exec_op();
    }
};

class op_projection: public Operators{
public:
    std::vector<std::shared_ptr<ast::Col>> cols;

    std::vector<std::shared_ptr<record>> exec_op(){
        std::vector<std::shared_ptr<record>> res;
        auto ret = next_node->exec_op();
        for(size_t i = 0; i < ret.size() ; i++){
            auto cur_record = ret[i];
            std::shared_ptr<record> new_record(new record);
            for(size_t j = 0; j < cur_record->row.size(); j++){
                auto cur_value = cur_record->row[j];
                for(size_t k = 0; k < cols.size(); k++){
                    if(cur_value->col_name == cols[k]->col_name)
                        new_record->row.push_back(cur_value);
                }
            }
            res.push_back(new_record);
        }

        return res;
    }
};

class op_selection: public Operators{
public:
    std::shared_ptr<ast::BinaryExpr> conds;
    
    std::vector<std::shared_ptr<record>> exec_op(){
        auto ret = next_node->exec_op();
        std::vector<std::shared_ptr<record>> res;

        if(auto x = std::dynamic_pointer_cast<ast::Col>(conds->rhs)){
            int lf_index = 0;
            int rg_index = 0;
            for(size_t i = 0; i < ret[0]->row.size(); i++){
                if(ret[0]->row[i]->tab_name == conds->lhs->tab_name && ret[0]->row[i]->col_name == conds->lhs->col_name){
                    lf_index = i;
                }
                if(ret[0]->row[i]->tab_name == x->tab_name && ret[0]->row[i]->col_name == x->col_name){
                    rg_index = i;
                }
            }

            for(size_t i = 0; i < ret.size(); i++){
                if(ret[i]->row[lf_index]->str == ret[i]->row[rg_index]->str){
                    res.push_back(ret[i]);
                }
            }
        }
        else if(auto x = std::dynamic_pointer_cast<ast::IntLit>(conds->rhs)){
            int lf_index = 0;
            for(size_t i = 0; i < ret[0]->row.size(); i++){
                if(ret[0]->row[i]->tab_name == conds->lhs->tab_name && ret[0]->row[i]->col_name == conds->lhs->col_name){
                    lf_index = i;
                }
            }
            
            for(size_t i = 0; i < ret.size(); i++){
                if(atoi(ret[i]->row[lf_index]->str.c_str()) == (x->val)){
                    res.push_back(ret[i]);
                }
            }
        }

        return res;
    }
};

class op_distribution: public Operators{
public:
    std::vector<std::shared_ptr<Operators>> nodes_plan;
    std::vector<std::string> nodes_name;

    std::vector<std::shared_ptr<record>> exec_op(){
        std::vector<std::shared_ptr<record>> res;

        for(size_t i = 0 ; i < nodes_plan.size(); i++){
            auto ret = send_plan(nodes_name[i],nodes_plan[i]);
            res.insert(res.end(), ret.begin(), ret.end());
        }

        return res;
    }
    
};

class op_join: public Operators{
public:
    std::vector<std::shared_ptr<Operators>> tables_get;

    std::vector<std::shared_ptr<record>> exec_op(){
        // 
        std::vector<std::vector<std::shared_ptr<record>>> ret;
        for(size_t i = 0; i < tables_get.size(); i++){
            ret.push_back(tables_get[i]->exec_op());
        }

        auto res = ret[0];
        std::vector<std::shared_ptr<record>> tmp; 

        for(size_t i = 1; i < ret.size(); i++){
            auto tab = ret[i];
            for(size_t j = 0; j < res.size(); j++){
                for(size_t k = 0; k < tab.size(); k++){
                    std::shared_ptr<record> join_record(new record);
                    join_record->row.insert(join_record->row.end(), res[j]->row.begin(), res[j]->row.end());
                    join_record->row.insert(join_record->row.end(), tab[k]->row.begin(), tab[k]->row.end());
                    tmp.push_back(join_record);
                }
            }
            res = tmp;
        }

        return res;
    }
};

class op_tablescan: public Operators{
public:
    std::string db_name;
    std::string tabs;
    int32_t par_id;

    std::vector<std::shared_ptr<record>> exec_op(){
        std::vector<std::shared_ptr<record>> res;

        for(auto i: kv->Key_range()){
            std::shared_ptr<record> red(new record);
            kv->get(i,red);
            res.push_back(red);
        }

        return res;
    }
};