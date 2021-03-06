#include <algorithm>
#include "polyhedronutils.h"
#include "Model/Element/Polyhedron.h"
#include "Model/Element/Polygon.h"
#include "Model/Element/Vertex.h"
#include "Model/Element/Tetrahedron.h"
#include "Utils/PolygonUtils.h"
PolyhedronUtils::PolyhedronUtils(){}
void addUniqueElement(vis::Vertex* vertex,std::vector<vis::Vertex*>& vertices){
	for( vis::Vertex* v : vertices )
		if(v == vertex)
			return;//already in vector
	vertices.push_back(vertex);
}

void PolyhedronUtils::getPolyhedronVertices(vis::Polyhedron* p, std::vector<vis::Vertex*>& ver){
	std::vector<vis::Polygon*>& polygons = p->getPolyhedronPolygons();
	for( vis::Polygon* polygon : polygons ){
		std::vector<vis::Vertex*>& vertices = polygon->getVertices();
		for( vis::Vertex* vertex : vertices )
			addUniqueElement(vertex,ver);
	}
}

void PolyhedronUtils::setPolyhedronRModelPositions(vis::Polyhedron* p){
	std::vector<int>& rmodelPos = p->getRmodelPositions();
	std::vector<vis::Polygon*>& polygons = p->getPolyhedronPolygons();
	for( vis::Polygon* polygon : polygons ){
		std::vector<int>& polygonRmodelPos = polygon->getRmodelPositions();
		for( int position : polygonRmodelPos)
			rmodelPos.push_back(position);
	}
}

void PolyhedronUtils::getTetrahedronIndices(vis::Polyhedron* p,
									  std::vector<GLuint>& tetrahedronIds) {
	std::vector<vis::Polygon*>& polygons = p->getPolyhedronPolygons();
	vis::Vertex* mainVertex = polygons[0]->getVertices()[0];
	for(std::vector<vis::Polygon*>::size_type i = 1;i<polygons.size();i++){
		std::vector<vis::Vertex*>& vertices = polygons[i]->getVertices();
		if(std::find(vertices.begin(), vertices.end(), mainVertex) == vertices.end()) {
			//std::vector<GLuint> trianglesIndices;
			//PolygonUtils::getTriangleIndices(polygons[i], trianglesIndices);

			for(std::vector<vis::Vertex*>::size_type j = 0; j < vertices.size();j+=3) {
				tetrahedronIds.push_back(mainVertex->getRmodelPositions()[0]);
				tetrahedronIds.push_back(vertices[j]->getRmodelPositions()[0]);
				tetrahedronIds.push_back(vertices[j+1]->getRmodelPositions()[0]);
				tetrahedronIds.push_back(vertices[j+2]->getRmodelPositions()[0]);
			}
		}

	}
}

float PolyhedronUtils::getPolyhedronSolidAngleFromVertex(vis::Polyhedron* p,
														 vis::Vertex* v){
	std::vector<vis::Polygon*> projectedFaces;
	std::vector<vis::Polygon*>& polygons = p->getPolyhedronPolygons();
	glm::vec3 geocenterToVertexV = v->getCoords()-p->getGeometricCenter();
	for( vis::Polygon* polygon : polygons ){
		if(glm::dot(p->getOutwardNormal(polygon),
					geocenterToVertexV)<0)
			projectedFaces.push_back(polygon);
	}
	float totalProjectedArea = 0.0f;
	for( vis::Polygon* polygon : projectedFaces ){
		std::vector<vis::Vertex*>& polygonVertices = polygon->getVertices();
		if(polygonVertices.size()<=2)
			continue;
		std::vector<glm::vec3> translatedCoords;
		for( vis::Vertex* vertex : polygonVertices )
			translatedCoords.push_back(glm::normalize(vertex->getCoords()-
													  v->getCoords()));
		//For each vertex, its coordinates are projeted to a sphere centered in v
		glm::vec3& v0(translatedCoords[0]);
		//divide polygon into triangles
		for(std::vector<vis::Vertex*>::size_type j = 1;
			j<polygonVertices.size()-1;j++){
			glm::vec3& v1(translatedCoords[j]);
			glm::vec3& v2(translatedCoords[j+1]);
			//get angle between vertices of the spherical triangle
			float angleA = std::acos(glm::dot(v0,v1)/( v0.length()*v1.length()));
			float angleB = std::acos(glm::dot(v0,v2)/( v0.length()*v2.length()));
			float angleC = std::acos(glm::dot(v1,v2)/( v1.length()*v2.length()));
			float angleAlfa = std::acos((std::cos(angleA)-(std::cos(angleB)*std::cos(angleC)))/
										(std::sin(angleB)*std::sin(angleC)));
			float ratio = std::sin(angleAlfa)/std::sin(angleA);
			float angleBeta = std::asin(ratio*std::sin(angleB));
			float angleGamma = std::asin(ratio*std::sin(angleC));
			float Et = angleAlfa + angleBeta + angleGamma - PI;//spherical excess in radians

			totalProjectedArea+=(Et/(4*PI));
		}
	}
	return totalProjectedArea;
}

glm::vec3 PolyhedronUtils::getCoordsSphericalCoordinates(glm::vec3 cartesian){
	glm::vec3 sphericalCoords;
	sphericalCoords.x = cartesian.length();
	if(cartesian.z==0)
		sphericalCoords.y = PI/2.0f;
	else if(cartesian.z<0)
		sphericalCoords.y = PI + std::atan(std::sqrt((cartesian.x*cartesian.x)+
													 (cartesian.y*cartesian.y))/cartesian.z);
	else
		sphericalCoords.y = std::atan(std::sqrt((cartesian.x*cartesian.x)+
												(cartesian.y*cartesian.y))/cartesian.z);

	if(cartesian.x>0&&cartesian.y>0)
		sphericalCoords.z = std::atan(cartesian.y/cartesian.x);
	else if(cartesian.x>0&&cartesian.y<0)
		sphericalCoords.z = 2*PI+std::atan(cartesian.y/cartesian.x);
	else if(cartesian.x==0)
		sphericalCoords.z = PI/2.0f*((cartesian.y > 0) - (cartesian.y < 0));
	else if(cartesian.x<0)
		sphericalCoords.z = PI+std::atan(cartesian.y/cartesian.x);
	return sphericalCoords;
}
