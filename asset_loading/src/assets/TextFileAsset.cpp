#include "TextFileAsset.h"

#include <fstream>
#include <sstream>

TextFileAsset::TextFileAsset(const std::string &path) : m_path(path){
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        m_content = stream.str();
    }catch(const std::ifstream::failure& e){
        throw std::invalid_argument("[Fatal Error] Could not read from file : [" + path + "]");
    }
}
