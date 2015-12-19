//
// Comparer.h
//
// Author: Ana Paula Mateo <anapaula.mateog@gmail.com >, (C) 2010
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>

using namespace std;


// Definiciones de constantes
//#define USE_FLANN
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

// Estructura de datos de correccion.
struct Posicion
{
	double x, y;
	double phi;
};

// CLASE COMPARADOR:	Realiza la comparaci�n entre imagenes consecutivas, calcula y
//						almacena el error en phi, x e y.
class Comparador
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DECLARACI�N DE VARIABLES
private:
	IplImage *actualImage, *oldImage;	// Imagenes actual y anterior con la que realiza la comparacion.
	IplImage *compensada;				// Imagen compensada.
	CvMemStorage* storage;				// Almac�n de memoria para las secuencias de caracter�sticas.
	CvPoint centro;						// Centro del rect�ngulo establecido para la comparaci�n.
	CvRect rect;						// Rect�ngulo de Inter�s para la comparaci�n.
	int ancho, alto;					// Alto y ancho del rect�ngulo de comparaci�n.
	Posicion pos;						// Posici�n relativa detectada entre la imagen anterior y la actual.
	char cad[50];						// Posici�n y orientaci�n relativa en modo texto.
	CvMat *m_rot, *m_tras;				// Matrices para la rotacion y la traslacion.
	CvPoint2D32f centro32f, a[3], b[3];	// Puntos de referencia para la rotacion y la traslacion.
	Posicion K;							// Ganancias del regulador tipo P.
	int umbralPhi;						// Umbral de descarte de phi.
//--------------------------------------------------------------------------------------------------------

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// M�TODOS P�BLICOS DE LA CLASE
public:

	// Constructor y destructor de la clase.
	Comparador(IplImage* init);
	~Comparador();
	//**************************************/

	// M�todos de INTERFAZ de la clase.
	Posicion getPos(){return pos;}	// Devuelve la posici�n relativa actual.
	char* getInformacion();			// Devuelve la posici�n relativa actual en modo texto.
	IplImage* getOldImage(){return oldImage;}			// Devuelve la imagen usada para comparar.
	IplImage* getActualImage(){return actualImage;}		// Devuelve la imagen actual.
	IplImage* getImagenCompensada(){return compensada;}	// Devuelve la imagen estabilizada.
	Posicion getGanancias(){return K;}		// Devuelve las ganancias utilizadas en el regulador P.
	int getUmbralPhi(){return umbralPhi;}	// Devuelve el umbral empleado para el �ngulo.

	void setOldImage(IplImage* old){oldImage = cvCloneImage(old);}				// Establece la imagen de referencia.
	void setActualImage(IplImage* actual){actualImage = cvCloneImage(actual);}	// Establece la imagen actual.
	void setCentro(CvPoint c){centro = c;}	// Establece el centro de la ROI de comparaci�n.
	void setDimensiones(int w, int h){alto=h; ancho=w;}		// Establece las dimensiones de la ROI.
	void setGanancias(double kphi, double kx, double ky)	// Establece las ganancias del regulador P.
	{ K.phi = kphi; K.x = kx; K.y = ky; }
	void setUmbralPhi(int umbral){umbralPhi = umbral;}		// Establece el umbral de �ngulo a emplear.

	void escribePos();	// Escribe en la l�nea de comandos la posicion relativa actual.
	//***************************************************************************************************/

	//	Metodos funcionales de la clase
    int calcular_errores();                     // Calcula la posici�n y orientaci�n relativas entre las im�genes.
    void compensarAngulo();                     // Corrige el �ngulo en la imagen actual.
    void compensarAngulo(IplImage* input);		// Corrige el �ngulo en la imagen que se le pasa.
    void compensarPosicion();                   // Corrigue la posici�n en la imagen actual.
	void compensarPosicion(IplImage* input);	// Corrigue el �ngulo en la imagen que se le pasa.
	
	void estabilizarImagen(IplImage* input);	// Calcula y realiza la estabilizaci�n de las im�genes con
												// los valores del regulador establecidos en K.
	void devuelveRegion(IplImage* input, IplImage* output, CvRect r);	// Devuelve la region de la imagen de entrada.
//----------------------------------------------------------------------------------------------------------

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//M�TODOS PRIVADOS DE LA CLASE
private:

	double calcular_angulo(CvPoint p);	// Devuelve el angulo formado por la recta que une p y el centro.
	double devuelveGrados(double phi){return phi*180/M_PI;}	// Devuelve el valor del �ngulo en grados.

	// Compara descriptores de puntos para evaluar su correspondencia.
	double compareSURFDescriptors( const float* d1, const float* d2, double best, int length );

	// Establece la correspondencia entre puntos de ambas im�genes.
	int naiveNearestNeighbor(	const float* vec, int laplacian, const CvSeq* model_keypoints,
								const CvSeq* model_descriptors );
	

	// Buscan parejas de puntos correspondientes.
#ifdef USE_FLANN
	void flannFindPairs(	const CvSeq*, const CvSeq* objectDescriptors,
							const CvSeq*, const CvSeq* imageDescriptors,
							vector<int>& ptpairs );
#else
	void findPairs( const CvSeq* objectKeypoints, const CvSeq* objectDescriptors,
					const CvSeq* imageKeypoints, const CvSeq* imageDescriptors,
					vector<int>& ptpairs );
#endif
	// Identifica objetos planos entre las secuencias de puntos correspondientes.
	int locatePlanarObject( const CvSeq* objectKeypoints, const CvSeq* objectDescriptors,
							const CvSeq* imageKeypoints, const CvSeq* imageDescriptors,
                    		const CvPoint src_corners[4], CvPoint dst_corners[4] );
};


