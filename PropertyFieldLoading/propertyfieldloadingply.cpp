#include "PropertyFieldLoading/propertyfieldloadingply.h"
#include "PropertyFieldLoading/scalarfielddef.h"
#include "Model/Element/Polygon.h"
#include "Model/Element/triangle.h"
#include "Model/Element/Vertex.h"
#include "Model/Element/edge.h"
#include "Model/PolygonMesh.h"
#include "Utils/fileutils.h"
#include "Utils/endianess.h"
#include "ModelLoading/modelloadingply.h"
#include "Exceptions/exceptionmessage.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unordered_map>
#include <regex>
PropertyFieldLoadingPly::PropertyFieldLoadingPly():PropertyFieldLoadingStrategy("PLY", "ply")
{
}
PropertyFieldLoadingPly::~PropertyFieldLoadingPly(){}


bool PropertyFieldLoadingPly::validate(std::string filename)
{
	return ModelLoadingPly().validate(filename);
}

std::vector<std::shared_ptr<PropertyFieldDef>> PropertyFieldLoadingPly::loadDefs(std::string filename)
{
	std::ifstream file(filename);
	if(!file.is_open()) {
		throw ExceptionMessage("Could not open file");
	}
	int np = 0;
	bool readingVertexProperties = false;
	//bool readingEdgeProperties = false;
	int propertyIndex = 0;
	std::vector<std::shared_ptr<PropertyFieldDef>> vertexProperties;
	std::string line;
	while(FileUtils::safeGetline(file, line)) {
		//std::istringstream iss(line);
		std::smatch sm;
		if( regex_match(line, sm,
						std::regex(R"(^\s*element\s+(\S+)\s+([0-9]+)\s*$)")) ) {
			readingVertexProperties = false;
			if(!sm[1].compare("vertex")) {
				readingVertexProperties = true;
				std::istringstream iss(sm[2]);
				iss >> np;
			}
		}
		else if ( regex_match (line, sm,
								std::regex(R"(^\s*property\s+(\S+)\s+(\S+)\s*$)")) ) {
			if(readingVertexProperties) {
				if(sm[2].compare("x") && sm[2].compare("y") && sm[2].compare("z")) { //property name
					std::string propertyName = sm[2];
					std::shared_ptr<ScalarFieldDef> newPtr(new ScalarFieldDef(propertyIndex, propertyName, np));
					vertexProperties.push_back(newPtr);
				}
				propertyIndex++;
			}
		} else if( !line.compare("end_header" ) ) {
			break;
		} else if ( regex_match (line, sm,
									std::regex(R"(^\s*format\s+(\S+)\s*$)")) ) {
			bool isBinary = std::string(sm[1]).find("ascii") != 0;
			if(isBinary) {
				throw ExceptionMessage("Can't read properties from binary format file");
			}
		}
	}
	return vertexProperties;
}

void PropertyFieldLoadingPly::load(std::string filename, Model* model,
								   std::vector<std::shared_ptr<PropertyFieldDef>> selectedProperties)
{
	try{
		this->filename = filename;
		this->selectedProperties = selectedProperties;
		model->accept((ModelVisitor*)this);
	}catch(std::bad_alloc &ba){
		throw std::bad_alloc();
	}catch(ModelLoadingException &ex){
		throw ex;
	}
}

void PropertyFieldLoadingPly::visit(VertexCloud* model) {
	readModelProperties(this->filename, model, this->selectedProperties);
}

void PropertyFieldLoadingPly::visit(PolygonMesh* model) {
	visit((VertexCloud*)model);
}

void PropertyFieldLoadingPly::visit(PolyhedronMesh* model) {
	visit((VertexCloud*) model);
}

bool PropertyFieldLoadingPly::readModelProperties( std::string filename, VertexCloud* vcloud, std::vector<std::shared_ptr<PropertyFieldDef>> selectedProperties) {
	std::vector<std::shared_ptr<ScalarFieldDef>> selectedScalars;
	for(std::shared_ptr<PropertyFieldDef> prop : selectedProperties) {
		selectedScalars.push_back(std::dynamic_pointer_cast<ScalarFieldDef>(prop));
	}
	std::ifstream file(filename);
	std::string line;
	// skip header
	while ( line.compare("end_header" ) ) {
		FileUtils::safeGetline(file, line);
	}
	auto totalProperties = selectedProperties.size();
	int totalProgress = 0;
	for(vis::Vertex* vertex : vcloud->getVertices()) {
		//propertyReader.setup(pol->getVertices()[i], startingIndex);
		// We assume properties are sorted by their id
		int currPropId = 0;
		auto currInsertionPropId = vcloud->getPropertyFieldDefs().size();
		FileUtils::safeGetline(file, line);
		std::istringstream iss(line);
		for(std::shared_ptr<ScalarFieldDef> prop : selectedScalars) {
			float value;
			while(currPropId <= prop->getId()){
				iss >> value;
				currPropId++;
			}
			//std::vector<std::shared_ptr<PropertyFieldDef>>::iterator it = find(propertyIds.begin(), propertyIds.end(), i);
			vertex->addScalarProperty(currInsertionPropId++, value);
			prop->expandBounds(value);
		}
		totalProgress+=totalProperties;
		if(totalProgress%1000 == 0) {
			emit setLoadedProgress(totalProgress);
		}
	}
	for(std::shared_ptr<PropertyFieldDef> prop : selectedProperties) {
		vcloud->addPropertyFieldDef(prop);
	}
	return true;
}

#include "Factories/PropertyFieldLoadingFactory.h"
REGISTER_PROPERTY_FIELD_LOADING_STRATEGY(PropertyFieldLoadingPly);
