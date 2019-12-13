// stdafx.cpp : source file that includes just the standard includes
// wndwsFormsGUI.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information
#include "stdafx.h"
#include "Form1.h"

//structure definitions
struct RangeData
{
	float highestX;
	float lowestX;
	float highestY;
	float lowestY;
	float highestZ;
	float lowestZ;
	int xRange;
	int yRange;
	int zRange;
};
struct Vectors
{
    std::vector<std::string> groupNames;
    std::vector<std::string> objectNames;
    std::vector<int> verticesPerObject;
};
struct OutputData
{
	int outputCount;
	int numRep;
	int currentValue;
	int currentDigits;
	int currentColumns;
};
	
//variable declarations
Vectors objectsAndVertices;
RangeData modelRange;
OutputData fout={0,0};

const int COLUMNS=3;
int numVertices=0, numFaces=0;



//function declarations
Vectors readOBJFile (std::string objFilePathU);
void plyHeaderInput (std::ifstream& plyFile, int& numVertices, int& numFaces);
void readVertices (std::ifstream& plyFile, RangeData& modelRange, float** vertices,/* float** normals, */int numVertices);
void modelNormalizing (RangeData modelRange, int numVertices, float** vertices);
void readFaces (std::ifstream& plyFile, long int** faces, int numFaces);
void voxelizeVertices (long int** faces, float** vertices, RangeData modelRange, int numVertices, int numFaces, Vectors objectsAndVertices);
void voxelizeEdges (float** vertices, long int** faces, int numFaces, int*** voxelArray, Vectors objectsAndVertices);
void voxelizeFaces (float** vertices, long int** faces, int faceVoxCount, int*** voxelArray, int objectNumber, Vectors objectsAndVertices);
void output (int***voxelArray, RangeData modelRange, Vectors objectsAndVertices);
void outputInfo(OutputData& fout, int*** voxelArray, int pCountX, int pCountY, int pCountZ);

//when "analyze" button is clicked, this function is executed
void wndwsFormsGUI::Form1::button1_Click(System::Object^ sender, System::EventArgs^ e)
{
	String^ objFilePath = textBox1->Text;
	std::string objFilePathU = msclr::interop::marshal_as<std::string>(objFilePath);
	
    objectsAndVertices=readOBJFile(objFilePathU);
	System::Windows::Forms::MessageBox::Show("obj file successfully parsed");
}

//when "voxelize!" button is clicked, this funcion is executed
void wndwsFormsGUI::Form1::button2_Click(System::Object^ sender, System::EventArgs^ e)
{
	String^ plyFilePath = textBox2->Text;
	std::string plyFilePathU = msclr::interop::marshal_as<std::string>(plyFilePath);

	std::wstring widestr = std::wstring(plyFilePathU.begin(), plyFilePathU.end());
	const wchar_t* widecstr = widestr.c_str();

	std::ifstream plyFile;
	plyFile.open(widecstr);

    if (!plyFile)
	{
		System::Windows::Forms::MessageBox::Show("no input ply file found to voxelize, make sure the file path and name are correct");
        //system("pause");
        //exit(1);
	}

	plyHeaderInput (plyFile, numVertices, numFaces);
	modelRange.highestX=-1000000;
	modelRange.highestY=-1000000;
	modelRange.highestZ=-1000000;
	modelRange.lowestX=1000000;
	modelRange.lowestY=1000000;
	modelRange.lowestZ=1000000;
	
	/*float** normals = new float*[numVertices];
	for (int i=0; i<numVertices; i++)
		normals[i] = new float[COLUMNS];*/
	float** vertices = new float*[numVertices];
	for (int j=0; j<numVertices; j++)
		vertices[j] = new float[COLUMNS];
	long int** faces = new long int*[numFaces];
	for (int k=0; k<numFaces; k++)
		faces[k] = new long int[COLUMNS+1];

	readVertices (plyFile, modelRange, vertices, /*normals, */numVertices);
	readFaces (plyFile, faces, numFaces);

	voxelizeVertices (faces, vertices, modelRange, numVertices, numFaces, objectsAndVertices);
	System::Windows::Forms::MessageBox::Show("Successfully voxelized CAD model");


	/*************OUTPUTTEST*************/
	/*std::ofstream file("C:/file.txt");*/
	/************************************/
}

//function definitions
Vectors readOBJFile(std::string objFilePathU)
{      
	std::wstring widestr = std::wstring(objFilePathU.begin(), objFilePathU.end());
	const wchar_t* widecstr = widestr.c_str();

	std::ifstream objFile;
	objFile.open(widecstr);
    Vectors functionVectors;
    if (!objFile)
	{
		System::Windows::Forms::MessageBox::Show("no input obj file found to analyze, make sure the file path and name are correct");
        //system("pause");
        //exit(1);
	}
	else
	{
		std::string fileInput;

	   int vertexCount=0, groupCount=0, objectCount=0, totalCount=1;
       
	   do
	   {
		   objFile>>fileInput;
		   if (fileInput=="g")
		   {
			   getline(objFile, fileInput);
			   groupCount++;
			   functionVectors.groupNames.resize(groupCount, fileInput);
			   vertexCount=0;
	               
			   objFile>>fileInput;
	               
			   if (fileInput=="o")
			   {
				   getline(objFile, fileInput);
				   objectCount++;
				   functionVectors.objectNames.resize(objectCount, fileInput);
				   while (fileInput!="f")
				   {
					   objFile>>fileInput;
					   if (fileInput=="v")
					   {
						   vertexCount++;
					   }
				   }
			   }
			   else
			   {
				   if (fileInput=="v")
					   vertexCount++;
	                   
				   while (fileInput!="f")
				   {
					   objFile>>fileInput;
					   if (fileInput=="v")
					   {
						   vertexCount++;
					   }
				   }
			   }
			   functionVectors.verticesPerObject.resize(totalCount, vertexCount);
			   totalCount++;
		   }
		   else if (fileInput=="o")
		   {	
			   getline(objFile, fileInput);
			   objectCount++;
			   functionVectors.objectNames.resize(objectCount, fileInput);
			   vertexCount=0;
			   while (fileInput!="f")
			   {
				   objFile>>fileInput;
				   if (fileInput=="v")
				   {
					   vertexCount++;
				   }
			   }
			   functionVectors.verticesPerObject.resize(totalCount, vertexCount);
			   totalCount++;
		   }
	   }while(objFile);
       
	   objFile.close();
   }
    return functionVectors;
}




void plyHeaderInput (std::ifstream& plyFile, int& numVertices, int& numFaces)
{
	std::string headerString;
    while(headerString!="end_header")
    {
        plyFile>>headerString;
        if (headerString=="element")
        {
            plyFile>>headerString;
            if (headerString=="vertex")
            {
                plyFile>>numVertices;
            }
            else if (headerString=="face")
            {
                plyFile>>numFaces;
            }
        }
    }
}
void readVertices (std::ifstream& plyFile, RangeData& modelRange, float** vertices, /*float** normals, */int numVertices)
{
	float fileNum=0.0;
    for (int fileCount=0; fileCount<numVertices; fileCount++)
    {
        for (int verticeCount=0; verticeCount<COLUMNS; verticeCount++)
        {
            plyFile>>fileNum;
            vertices[fileCount][verticeCount]=fileNum;
            if (verticeCount==0 && fileNum>modelRange.highestX)
            {
                modelRange.highestX=fileNum;
            }
            else if (verticeCount==1 && fileNum>modelRange.highestY)
            {
                modelRange.highestY=fileNum;
            }
            else if (verticeCount==2 && fileNum>modelRange.highestZ)
            {
                modelRange.highestZ=fileNum;
            }
            
            if (verticeCount==0 && fileNum<modelRange.lowestX)
            {
                modelRange.lowestX=fileNum;
            }
            else if (verticeCount==1 && fileNum<modelRange.lowestY)
            {
                modelRange.lowestY=fileNum;
            }
            else if (verticeCount==2 && fileNum<modelRange.lowestZ)
            {
                modelRange.lowestZ=fileNum;
            }
        }
		
        for (int normalCount=0; normalCount<COLUMNS; normalCount++)
        {
            plyFile>>fileNum;
            //normals[fileCount][normalCount]=fileNum;
        }
    }
}
void readFaces (std::ifstream& plyFile, long int** faces, int numFaces)
{
	int verticesOnThisFace=0;
	long int faceNumber=0;
    for (int fileCount=0; fileCount<numFaces; fileCount++)
    {
        plyFile>>verticesOnThisFace;
        for (int faceCount=0; faceCount<verticesOnThisFace; faceCount++)
        {
            plyFile>>faceNumber;
            faces[fileCount][faceCount]=faceNumber;
        }
        if (verticesOnThisFace==COLUMNS)
        {
            faces[fileCount][COLUMNS]=-1;
        }
    }
}
void voxelizeVertices (long int** faces, float** vertices, RangeData modelRange, int numVertices, int numFaces, Vectors objectsAndVertices)
{
	modelRange.xRange=modelRange.lowestX-modelRange.highestX;
	modelRange.yRange=modelRange.lowestY-modelRange.highestY;
	modelRange.zRange=modelRange.lowestZ-modelRange.highestZ;
	if (modelRange.xRange<0)									//Note, I didn't use the absolute value function (fabs( )) because I was getting a strange error,
		modelRange.xRange=modelRange.xRange*-1;					//hence I manually calculated the absolute value of the ranges.
	if (modelRange.yRange<0)
		modelRange.yRange=modelRange.yRange*-1;
	if (modelRange.zRange<0)
		modelRange.zRange=modelRange.zRange*-1;

	modelRange.xRange+=1;												//have to add 1 to the ranges because a few lines up, floats are converted to ints, which means
	modelRange.yRange+=1;												//something like 5.8 becomes 5. the extra 0.8 may cause problems later on, so lets just add 1 and 
	modelRange.zRange+=1;												//not worry about it.		

	modelNormalizing (modelRange, numVertices, vertices);

    //dynamic allocation of 3D voxel array
    int*** voxelArray=new int** [modelRange.xRange+1]();
    for (int i=0; i<(modelRange.xRange+1); i++)
    {
        voxelArray[i]=new int* [modelRange.yRange+1]();
        for (int j=0; j<(modelRange.yRange+1); j++)
        {
            voxelArray[i][j]=new int [modelRange.zRange+1]();
        }
    }
    
    int vertexInc=0;
	
    for (int objectCount=0; objectCount<objectsAndVertices.groupNames.size(); objectCount++)
    {
        for (int voxCount=0; voxCount<objectsAndVertices.verticesPerObject[objectCount]; voxCount++)
		{
			voxelArray[(int)floor(vertices[vertexInc][COLUMNS-3]+0.5)][(int)floor(vertices[vertexInc][COLUMNS-2]+0.5)][(int)floor(vertices[vertexInc][COLUMNS-1]+0.5)]=(objectCount+1);
            vertexInc++;
        }
    }

	
    voxelizeEdges(vertices, faces, numFaces, voxelArray, objectsAndVertices);
   
    output (voxelArray, modelRange, objectsAndVertices);
    
    //deallocating voxelArray
    for(int l=0;l<(modelRange.xRange+1);l++)
    {
        for(int m=0;m<(modelRange.yRange+1);m++)
        {
            delete[] voxelArray[l][m];
        }
        delete[] voxelArray[l];
    }
    delete [] voxelArray;
    
}
void modelNormalizing (RangeData modelRange, int numVertices, float** vertices)
{
	if (modelRange.lowestX != 0)
	{
		if (modelRange.lowestX<0)
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-3]+=fabs(modelRange.lowestX);
			}
		}
		else
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-3]-=fabs(modelRange.lowestX);
			}
		}
	}

	if (modelRange.lowestY != 0)
	{
		if (modelRange.lowestY<0)
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-2]+=fabs(modelRange.lowestY);
			}
		}
		else
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-2]-=fabs(modelRange.lowestY);
			}
		}
	}

	if (modelRange.lowestZ != 0)
	{
		if (modelRange.lowestZ<0)
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-1]+=fabs(modelRange.lowestZ);
			}
		}
		else
		{
			for (int adjustCount=0; adjustCount<numVertices; adjustCount++)
			{
				vertices[adjustCount][COLUMNS-1]-=fabs(modelRange.lowestZ);
			}
		}
	}
}
void voxelizeEdges (float** vertices, long int** faces, int numFaces, int*** voxelArray, Vectors objectsAndVertices)
{
	float unitVectorEdge1[COLUMNS], unitVectorEdge2[COLUMNS],
          unitVectorEdge3[COLUMNS], unitVectorEdge4[COLUMNS];
    
    
    for (int edgeVoxCount=0; edgeVoxCount<numFaces; edgeVoxCount++)
    {
		int augmentVertex=objectsAndVertices.verticesPerObject[0];
		int objectNumber=1;
        //determining which object it is
        while ((faces[edgeVoxCount][0])>=augmentVertex)
        {
        	augmentVertex+=objectsAndVertices.verticesPerObject[objectNumber];
            objectNumber++;
        }

        bool reachedVertice=false;
        float* edge1dir= new float[COLUMNS];
        float* edge2dir= new float[COLUMNS];
        float* edge3dir= new float[COLUMNS];
        float* edge4dir= new float[COLUMNS];
        
        if (faces[edgeVoxCount][COLUMNS]!=-1)
        {
            for (int direcCount=0; direcCount<COLUMNS; direcCount++)
            {
                edge1dir[direcCount]=vertices[faces[edgeVoxCount][1]][direcCount]-vertices[faces[edgeVoxCount][0]][direcCount];
                edge2dir[direcCount]=vertices[faces[edgeVoxCount][2]][direcCount]-vertices[faces[edgeVoxCount][1]][direcCount];
                edge3dir[direcCount]=vertices[faces[edgeVoxCount][3]][direcCount]-vertices[faces[edgeVoxCount][2]][direcCount];
                edge4dir[direcCount]=vertices[faces[edgeVoxCount][0]][direcCount]-vertices[faces[edgeVoxCount][3]][direcCount];
            }
            float *normalizingFactors=new float [COLUMNS+1];
            
            normalizingFactors[0]=sqrt(edge1dir[0]*edge1dir[0]+edge1dir[1]*edge1dir[1]+edge1dir[2]*edge1dir[2]);
            normalizingFactors[1]=sqrt(edge2dir[0]*edge2dir[0]+edge2dir[1]*edge2dir[1]+edge2dir[2]*edge2dir[2]);
            normalizingFactors[2]=sqrt(edge3dir[0]*edge3dir[0]+edge3dir[1]*edge3dir[1]+edge3dir[2]*edge3dir[2]);
            normalizingFactors[3]=sqrt(edge4dir[0]*edge4dir[0]+edge4dir[1]*edge4dir[1]+edge4dir[2]*edge4dir[2]);
            
            
            //calculating unit vectors for each edge
            
            for (int normalizeCount=0; normalizeCount<COLUMNS; normalizeCount++)
            {
                unitVectorEdge1[normalizeCount]=(edge1dir[normalizeCount]/normalizingFactors[0]);
                unitVectorEdge2[normalizeCount]=(edge2dir[normalizeCount]/normalizingFactors[1]);
                unitVectorEdge3[normalizeCount]=(edge3dir[normalizeCount]/normalizingFactors[2]);
                unitVectorEdge4[normalizeCount]=(edge4dir[normalizeCount]/normalizingFactors[3]);
            }
            
            //edge1 voxelization loop
            reachedVertice=false;
            int voxCount=1;
            float voxIndexX=0, voxIndexY=0, voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][0]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][0]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][0]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][1]][0]-voxIndexX)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][1]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][1]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][1]][0]-voxIndexX)>1)
                        voxIndexX+=unitVectorEdge1[0];
                        if (fabs(vertices[faces[edgeVoxCount][1]][1]-voxIndexY)>1)
                        voxIndexY+=unitVectorEdge1[1];
                        if (fabs(vertices[faces[edgeVoxCount][1]][2]-voxIndexZ)>1)
                        voxIndexZ+=unitVectorEdge1[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            
            //edge2 voxelization loop
            reachedVertice=false;
            voxCount=1;
            voxIndexX=0;
            voxIndexY=0;
            voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][1]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][1]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][1]][2];
                    }
                if (fabs(vertices[faces[edgeVoxCount][2]][0]-voxIndexX)<=1 &&
                    fabs(vertices[faces[edgeVoxCount][2]][1]-voxIndexY)<=1 &&
                    fabs(vertices[faces[edgeVoxCount][2]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][2]][0]-voxIndexX)>1)
                            voxIndexX+=unitVectorEdge2[0];
                        if (fabs(vertices[faces[edgeVoxCount][2]][1]-voxIndexY)>1)
                            voxIndexY+=unitVectorEdge2[1];
                        if (fabs(vertices[faces[edgeVoxCount][2]][2]-voxIndexZ)>1)
                            voxIndexZ+=unitVectorEdge2[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            
            //edge3 voxelization loop
            reachedVertice=false;
            voxCount=1;
            voxIndexX=0;
            voxIndexY=0;
            voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][2]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][2]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][2]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][3]][0]-voxIndexX)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][3]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][3]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][3]][0]-voxIndexX)>1)
                            voxIndexX+=unitVectorEdge3[0];
                        if (fabs(vertices[faces[edgeVoxCount][3]][1]-voxIndexY)>1)
                            voxIndexY+=unitVectorEdge3[1];
                        if (fabs(vertices[faces[edgeVoxCount][3]][2]-voxIndexZ)>1)
                            voxIndexZ+=unitVectorEdge3[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            
            //edge4 voxelization loop
            reachedVertice=false;
            voxCount=1;
            voxIndexX=0;
            voxIndexY=0;
            voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][3]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][3]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][3]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][0]][0]-voxIndexX)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][0]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][0]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][0]][0]-voxIndexX)>1)
                        voxIndexX+=unitVectorEdge4[0];
                        if (fabs(vertices[faces[edgeVoxCount][0]][1]-voxIndexY)>1)
                        voxIndexY+=unitVectorEdge4[1];
                        if (fabs(vertices[faces[edgeVoxCount][0]][2]-voxIndexZ)>1)
                        voxIndexZ+=unitVectorEdge4[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            delete [] edge1dir;
            delete [] edge2dir;
            delete [] edge3dir;
            delete [] edge4dir;
            delete [] normalizingFactors;
         }
        else if (faces[edgeVoxCount][COLUMNS]==-1)
        {
            for (int direcCount=0; direcCount<COLUMNS; direcCount++)
            {
                edge1dir[direcCount]=vertices[faces[edgeVoxCount][1]][direcCount]-vertices[faces[edgeVoxCount][0]][direcCount];
                edge2dir[direcCount]=vertices[faces[edgeVoxCount][2]][direcCount]-vertices[faces[edgeVoxCount][1]][direcCount];
                edge3dir[direcCount]=vertices[faces[edgeVoxCount][0]][direcCount]-vertices[faces[edgeVoxCount][2]][direcCount];
            }
            
            float *normalizingFactors=new float [COLUMNS];
            
            normalizingFactors[0]=sqrt(edge1dir[0]*edge1dir[0]+edge1dir[1]*edge1dir[1]+edge1dir[2]*edge1dir[2]);
            normalizingFactors[1]=sqrt(edge2dir[0]*edge2dir[0]+edge2dir[1]*edge2dir[1]+edge2dir[2]*edge2dir[2]);
            normalizingFactors[2]=sqrt(edge3dir[0]*edge3dir[0]+edge3dir[1]*edge3dir[1]+edge3dir[2]*edge3dir[2]);
            
            //calculating unit vectors for each edge
            for (int normalizeCount=0; normalizeCount<COLUMNS; normalizeCount++)
            {
                unitVectorEdge1[normalizeCount]=(edge1dir[normalizeCount]/normalizingFactors[0]);
                unitVectorEdge2[normalizeCount]=(edge2dir[normalizeCount]/normalizingFactors[1]);
                unitVectorEdge3[normalizeCount]=(edge3dir[normalizeCount]/normalizingFactors[2]);
            }
            //edge1 voxelization loop
            reachedVertice=false;
            int voxCount=1;
            float voxIndexX=0, voxIndexY=0, voxIndexZ=0;
            
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][0]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][0]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][0]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][1]][0]-voxIndexX)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][1]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][1]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][1]][0]-voxIndexX)>1)
                            voxIndexX+=unitVectorEdge1[0];
                        if (fabs(vertices[faces[edgeVoxCount][1]][1]-voxIndexY)>1)
                            voxIndexY+=unitVectorEdge1[1];
                        if (fabs(vertices[faces[edgeVoxCount][1]][2]-voxIndexZ)>1)
                            voxIndexZ+=unitVectorEdge1[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            
            //edge2 voxelization loop
            reachedVertice=false;
            voxCount=1;
            voxIndexX=0;
            voxIndexY=0;
            voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][1]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][1]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][1]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][2]][0]-voxIndexX)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][2]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][2]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][2]][0]-voxIndexX)>1)
                            voxIndexX+=unitVectorEdge2[0];
                        if (fabs(vertices[faces[edgeVoxCount][2]][1]-voxIndexY)>1)
                            voxIndexY+=unitVectorEdge2[1];
                        if (fabs(vertices[faces[edgeVoxCount][2]][2]-voxIndexZ)>1)
                            voxIndexZ+=unitVectorEdge2[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            
            //edge3 voxelization loop
            reachedVertice=false;
            voxCount=1;
            voxIndexX=0;
            voxIndexY=0;
            voxIndexZ=0;
            while (reachedVertice==false)
            {
                    if (voxCount==1)
                    {
                        voxIndexX=vertices[faces[edgeVoxCount][2]][0];
                        voxIndexY=vertices[faces[edgeVoxCount][2]][1];
                        voxIndexZ=vertices[faces[edgeVoxCount][2]][2];
                    }
                    if (fabs(vertices[faces[edgeVoxCount][0]][0]-voxIndexX)<=1 &&
                       fabs(vertices[faces[edgeVoxCount][0]][1]-voxIndexY)<=1 &&
                        fabs(vertices[faces[edgeVoxCount][0]][2]-voxIndexZ)<=1)
                    {
                        reachedVertice=true;
                    }
                    else
                    {
                        if (fabs(vertices[faces[edgeVoxCount][0]][0]-voxIndexX)>1)
                        voxIndexX+=unitVectorEdge3[0];
                        if (fabs(vertices[faces[edgeVoxCount][0]][1]-voxIndexY)>1)
                        voxIndexY+=unitVectorEdge3[1];
                        if (fabs(vertices[faces[edgeVoxCount][0]][2]-voxIndexZ)>1)
                        voxIndexZ+=unitVectorEdge3[2];
                        voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
                    }
                voxCount++;
            }
            delete [] edge1dir;
            delete [] edge2dir;
            delete [] edge3dir;
            delete [] edge4dir;
            delete [] normalizingFactors;
        }
            voxelizeFaces (vertices, faces, edgeVoxCount, voxelArray, objectNumber, objectsAndVertices);
    }
}
void voxelizeFaces (float** vertices, long int** faces, int faceVoxCount, int*** voxelArray, int objectNumber, Vectors objectsAndVertices)
{
    /* **************************************************************************************************** */
    /*  This is the naming convention I am using for the vertices in each polygon.                          */
    /*  Note that ply files and obj files automatically list vertices in counter-clockwise order.           */
    /*                                                                         3                            */
    /*                                                                        / \                           */
    /*            4-------------3                                            /   \                          */
    /*            |             |                                           /     \                         */
    /*            |             |                                          /       \                        */
    /*            |             |                                         /         \                       */
    /*            |             |                                        /           \                      */
    /*            1-------------2                                       1-------------2                     */
    /*                                                                                                      */
    /* **************************************************************************************************** */
     
    
    if (faces[faceVoxCount][COLUMNS]==-1)
    {
        float* vert1to3= new float [COLUMNS];
        float* vert2to3= new float [COLUMNS];
        
        for (int direcCount=0; direcCount<COLUMNS; direcCount++)
        {
            vert1to3[direcCount]=vertices[faces[faceVoxCount][2]][direcCount]-vertices[faces[faceVoxCount][0]][direcCount];
            vert2to3[direcCount]=vertices[faces[faceVoxCount][2]][direcCount]-vertices[faces[faceVoxCount][1]][direcCount];
        }
        float normalizingFactors[COLUMNS-1];
        
        normalizingFactors[0]=sqrt(vert1to3[0]*vert1to3[0]+vert1to3[1]*vert1to3[1]+vert1to3[2]*vert1to3[2]);
        normalizingFactors[1]=sqrt(vert2to3[0]*vert2to3[0]+vert2to3[1]*vert2to3[1]+vert2to3[2]*vert2to3[2]);
        
        float unitVector1to3[COLUMNS], unitVector2to3[COLUMNS];
        
        for (int uVectCount=0; uVectCount<COLUMNS; uVectCount++)
        {
            unitVector1to3[uVectCount]=vert1to3[uVectCount]/normalizingFactors[0];
            unitVector2to3[uVectCount]=vert2to3[uVectCount]/normalizingFactors[1];
        }
        //voxelizing the triangle
        bool reachedVertice=false;
        float voxIndexX=0;
        float voxIndexY=0;
        float voxIndexZ=0;
        float limiterX=0, limiterY=0, limiterZ=0;
        float varCoordX=0, varCoordY=0, varCoordZ=0;
        bool reachedVariableVertice=false;
        int voxCount=1;
        while (reachedVertice==false)
        {
            if (voxCount==1)
            {
                voxIndexX=vertices[faces[faceVoxCount][0]][0];
                voxIndexY=vertices[faces[faceVoxCount][0]][1];
                voxIndexZ=vertices[faces[faceVoxCount][0]][2];
                limiterX=vertices[faces[faceVoxCount][1]][0];
                limiterY=vertices[faces[faceVoxCount][1]][1];
                limiterZ=vertices[faces[faceVoxCount][1]][2];
            }
            if ((fabs(vertices[faces[faceVoxCount][2]][0]-voxIndexX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][2]][1]-voxIndexY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][2]][2]-voxIndexZ)<=1 )||(
                 fabs(vertices[faces[faceVoxCount][2]][0]-limiterX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][2]][1]-limiterY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][2]][2]-limiterZ)<=1))
            {
                reachedVertice=true;
            }
            else
            {
                voxIndexX+=unitVector1to3[0];
                voxIndexY+=unitVector1to3[1];
                voxIndexZ+=unitVector1to3[2];
                limiterX+=unitVector2to3[0];
                limiterY+=unitVector2to3[1];
                limiterZ+=unitVector2to3[2];
                
                varCoordX=voxIndexX;
                varCoordY=voxIndexY;
                varCoordZ=voxIndexZ;
                
                float* variableDirection=new float [COLUMNS];
                
                variableDirection[0]=limiterX-varCoordX;
                variableDirection[1]=limiterY-varCoordY;
                variableDirection[2]=limiterZ-varCoordZ;
                
                float normalizingFactor=0;
                
                normalizingFactor=sqrt(variableDirection[0]*variableDirection[0]+variableDirection[1]*variableDirection[1]+
                                       variableDirection[2]*variableDirection[2]);
                
                float* variableUnitVector=new float [COLUMNS];
                
                for (int unitVectCount=0; unitVectCount<COLUMNS; unitVectCount++)
                {
                    variableUnitVector[unitVectCount]=variableDirection[unitVectCount]/normalizingFactor;
                }
                reachedVariableVertice=false;
                while (reachedVariableVertice==false)
                {
                    if (fabs(limiterX-varCoordX)<=1 &&
                        fabs(limiterY-varCoordY)<=1 &&
                        fabs(limiterZ-varCoordZ)<=1)
                    {
                        reachedVariableVertice=true;
                    }
                    else
                    {
                        varCoordX+=variableUnitVector[0];
                        varCoordY+=variableUnitVector[1];
                        varCoordZ+=variableUnitVector[2];
                        voxelArray[(int)floor(varCoordX+0.5)][(int)floor(varCoordY+0.5)][(int)floor(varCoordZ+0.5)]=objectNumber;
                    }
                }
                delete [] variableDirection;
                delete [] variableUnitVector;
            }
            voxCount++;
        }
        
        delete [] vert1to3;
        delete [] vert2to3;
    }
    else if (faces[faceVoxCount][COLUMNS]!=-1)
    {
        //voxelizing faces with 4 vertices.
        /*
            this method is going to split the polygon in half via a line
            connecting the first vertex with the third vertex, creating 2 triangles
            within the polygon, then voxelizing those triangles like the method above (LINES _-_)
        */
        
        float* diagLine= new float [COLUMNS]; //direction vector pointing from vertex 1 to 3.
        float* vert1to2= new float [COLUMNS]; //direction vector pointing from vertex 1 to 2.
        float* vert3to2= new float [COLUMNS]; //direction vector pointing from vertex 3 to 2.
        float* vert1to4= new float [COLUMNS]; //direction vector pointing from vertex 1 to 4.
        float* vert3to4= new float [COLUMNS]; //direction vector pointing from vertex 3 to 4.
        
        for (int direcCount=0; direcCount<COLUMNS; direcCount++)
        {
            diagLine[direcCount]=vertices[faces[faceVoxCount][2]][direcCount]-vertices[faces[faceVoxCount][0]][direcCount];
            vert1to2[direcCount]=vertices[faces[faceVoxCount][1]][direcCount]-vertices[faces[faceVoxCount][0]][direcCount];
            vert3to2[direcCount]=vertices[faces[faceVoxCount][1]][direcCount]-vertices[faces[faceVoxCount][2]][direcCount];
            vert1to4[direcCount]=vertices[faces[faceVoxCount][3]][direcCount]-vertices[faces[faceVoxCount][0]][direcCount];
            vert3to4[direcCount]=vertices[faces[faceVoxCount][3]][direcCount]-vertices[faces[faceVoxCount][2]][direcCount];
        }
        
        float normalizingFactors [COLUMNS+2]={0};
        
        normalizingFactors[0]=sqrt(diagLine[0]*diagLine[0]+diagLine[1]*diagLine[1]+diagLine[2]*diagLine[2]);
        normalizingFactors[1]=sqrt(vert1to2[0]*vert1to2[0]+vert1to2[1]*vert1to2[1]+vert1to2[2]*vert1to2[2]);
        normalizingFactors[2]=sqrt(vert3to2[0]*vert3to2[0]+vert3to2[1]*vert3to2[1]+vert3to2[2]*vert3to2[2]);
        normalizingFactors[3]=sqrt(vert1to4[0]*vert1to4[0]+vert1to4[1]*vert1to4[1]+vert1to4[2]*vert1to4[2]);
        normalizingFactors[4]=sqrt(vert3to4[0]*vert3to4[0]+vert3to4[1]*vert3to4[1]+vert3to4[2]*vert3to4[2]);
        
        //calculating unit vectors (arrays)
        float unitVectorDiag[COLUMNS], unitVector1to2[COLUMNS], unitVector3to2[COLUMNS], unitVector1to4[COLUMNS],
               unitVector3to4[COLUMNS];
        
        for (int uVectCalc=0; uVectCalc<COLUMNS; uVectCalc++)
        {
            unitVectorDiag[uVectCalc]=(diagLine[uVectCalc]/normalizingFactors[0]);
            unitVector1to2[uVectCalc]=(vert1to2[uVectCalc]/normalizingFactors[1]);
            unitVector3to2[uVectCalc]=(vert3to2[uVectCalc]/normalizingFactors[2]);
            unitVector1to4[uVectCalc]=(vert1to4[uVectCalc]/normalizingFactors[3]);
            unitVector3to4[uVectCalc]=(vert3to4[uVectCalc]/normalizingFactors[4]);
        }
        //voxelizing diagonal line
        float voxIndexX=0, voxIndexY=0, voxIndexZ=0;
        bool reachedVertice=false;
        int voxCount=1;
        while (reachedVertice==false)
        {
            if (voxCount==1)
            {
                voxIndexX=vertices[faces[faceVoxCount][0]][0];
                voxIndexY=vertices[faces[faceVoxCount][0]][1];
                voxIndexZ=vertices[faces[faceVoxCount][0]][2];
            }
            if (fabs(vertices[faces[faceVoxCount][2]][0]-voxIndexX)<=1 &&
                fabs(vertices[faces[faceVoxCount][2]][1]-voxIndexY)<=1 &&
                fabs(vertices[faces[faceVoxCount][2]][2]-voxIndexZ)<=1)
            {
                reachedVertice=true;
            }
            else
            {
                if (fabs(vertices[faces[faceVoxCount][2]][0]-voxIndexX)>1)
                voxIndexX+=unitVectorDiag[0];
                if (fabs(vertices[faces[faceVoxCount][2]][1]-voxIndexY)>1)
                voxIndexY+=unitVectorDiag[1];
                if (fabs(vertices[faces[faceVoxCount][2]][2]-voxIndexZ)>1)
                voxIndexZ+=unitVectorDiag[2];
                voxelArray[(int)floor(voxIndexX+0.5)][(int)floor(voxIndexY+0.5)][(int)floor(voxIndexZ+0.5)]=objectNumber;
            }
            voxCount++;
        }
        
        //voxelizing triangle with vertices 1 3 4
        reachedVertice=false;
        voxIndexX=0;
        voxIndexY=0;
        voxIndexZ=0;
        float limiterX=0, limiterY=0, limiterZ=0;
        float varCoordX=0, varCoordY=0, varCoordZ=0;
        bool reachedVariableVertice=false;
        voxCount=1;
        while (reachedVertice==false)
        {
            if (voxCount==1)
            {
                voxIndexX=vertices[faces[faceVoxCount][0]][0];
                voxIndexY=vertices[faces[faceVoxCount][0]][1];
                voxIndexZ=vertices[faces[faceVoxCount][0]][2];
                limiterX=vertices[faces[faceVoxCount][2]][0];
                limiterY=vertices[faces[faceVoxCount][2]][1];
                limiterZ=vertices[faces[faceVoxCount][2]][2];
            }
            if ((fabs(vertices[faces[faceVoxCount][3]][0]-voxIndexX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][3]][1]-voxIndexY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][3]][2]-voxIndexZ)<=1 )||(
                 fabs(vertices[faces[faceVoxCount][3]][0]-limiterX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][3]][1]-limiterY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][3]][2]-limiterZ)<=1))
            {
                reachedVertice=true;
            }
            else
            {
                voxIndexX+=unitVector1to4[0];
                voxIndexY+=unitVector1to4[1];
                voxIndexZ+=unitVector1to4[2];
                limiterX+=unitVector3to4[0];
                limiterY+=unitVector3to4[1];
                limiterZ+=unitVector3to4[2];
                
                varCoordX=voxIndexX;
                varCoordY=voxIndexY;
                varCoordZ=voxIndexZ;
                
                float* variableDirection=new float [COLUMNS];
                
                variableDirection[0]=limiterX-varCoordX;
                variableDirection[1]=limiterY-varCoordY;
                variableDirection[2]=limiterZ-varCoordZ;
                
                float normalizingFactor=0;
                
                normalizingFactor=sqrt(variableDirection[0]*variableDirection[0]+variableDirection[1]*variableDirection[1]+
                                       variableDirection[2]*variableDirection[2]);
                
                float* variableUnitVector=new float [COLUMNS];
                
                for (int unitVectCount=0; unitVectCount<COLUMNS; unitVectCount++)
                {
                    variableUnitVector[unitVectCount]=variableDirection[unitVectCount]/normalizingFactor;
                }
                reachedVariableVertice=false;
                while (reachedVariableVertice==false)
                {
                    if (fabs(limiterX-varCoordX)<=1 &&
                        fabs(limiterY-varCoordY)<=1 &&
                        fabs(limiterZ-varCoordZ)<=1)
                    {
                        reachedVariableVertice=true;
                    }
                    else
                    {
                        varCoordX+=variableUnitVector[0];
                        varCoordY+=variableUnitVector[1];
                        varCoordZ+=variableUnitVector[2];
                        voxelArray[(int)floor(varCoordX+0.5)][(int)floor(varCoordY+0.5)][(int)floor(varCoordZ+0.5)]=objectNumber;
                    }
                }
                delete [] variableDirection;
                delete [] variableUnitVector;
            }
            voxCount++;
        }
        //voxelizing triangle with vertices 1 2 3
        reachedVertice=false;
        voxIndexX=0;
        voxIndexY=0;
        voxIndexZ=0;
        limiterX=0;
        limiterY=0;
        limiterZ=0;
        varCoordX=0;
        varCoordY=0;
        varCoordZ=0;
        reachedVariableVertice=false;
        voxCount=1;
        
        while (reachedVertice==false)
        {
            if (voxCount==1)
            {
                voxIndexX=vertices[faces[faceVoxCount][0]][0];
                voxIndexY=vertices[faces[faceVoxCount][0]][1];
                voxIndexZ=vertices[faces[faceVoxCount][0]][2];
                limiterX=vertices[faces[faceVoxCount][2]][0];
                limiterY=vertices[faces[faceVoxCount][2]][1];
                limiterZ=vertices[faces[faceVoxCount][2]][2];
            }
            if ((fabs(vertices[faces[faceVoxCount][1]][0]-voxIndexX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][1]][1]-voxIndexY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][1]][2]-voxIndexZ)<=1 )||(
                 fabs(vertices[faces[faceVoxCount][1]][0]-limiterX)<=1 &&
                 fabs(vertices[faces[faceVoxCount][1]][1]-limiterY)<=1 &&
                 fabs(vertices[faces[faceVoxCount][1]][2]-limiterZ)<=1))
            {
                reachedVertice=true;
            }
            else
            {
                voxIndexX+=unitVector1to2[0];
                voxIndexY+=unitVector1to2[1];
                voxIndexZ+=unitVector1to2[2];
                limiterX+=unitVector3to2[0];
                limiterY+=unitVector3to2[1];
                limiterZ+=unitVector3to2[2];
                
                varCoordX=voxIndexX;
                varCoordY=voxIndexY;
                varCoordZ=voxIndexZ;
                
                float* variableDirection=new float [COLUMNS];
                variableDirection[0]=limiterX-varCoordX;
                variableDirection[1]=limiterY-varCoordY;
                variableDirection[2]=limiterZ-varCoordZ;
                float normalizingFactor=0;
                normalizingFactor=sqrt(variableDirection[0]*variableDirection[0]+variableDirection[1]*variableDirection[1]+
                                       variableDirection[2]*variableDirection[2]);
                float* variableUnitVector=new float [COLUMNS];
                
                for (int unitVectCount=0; unitVectCount<COLUMNS; unitVectCount++)
                {
                    variableUnitVector[unitVectCount]=variableDirection[unitVectCount]/normalizingFactor;
                }
                reachedVariableVertice=false;
                while (reachedVariableVertice==false)
                {
                    if (fabs(limiterX-varCoordX)<=1 &&
                        fabs(limiterY-varCoordY)<=1 &&
                        fabs(limiterZ-varCoordZ)<=1)
                    {
                        reachedVariableVertice=true;
                    }
                    else
                    {
                        varCoordX+=variableUnitVector[0];
                        varCoordY+=variableUnitVector[1];
                        varCoordZ+=variableUnitVector[2];
                        voxelArray[(int)floor(varCoordX+0.5)][(int)floor(varCoordY+0.5)][(int)floor(varCoordZ+0.5)]=objectNumber;
                    }
                }
                delete [] variableDirection;
                delete [] variableUnitVector;
            }
            voxCount++;
        }
        
        
        delete [] diagLine;
        delete [] vert1to2;
        delete [] vert3to2;
        delete [] vert1to4;
        delete [] vert3to4;
    }
}
void output (int***voxelArray, RangeData modelRange, Vectors objectsAndVertices)
{
	std::ofstream file("C:/file.txt");
	
	file<<"c this input file was made with a CAD2Voxel algorithm\nc originally created by Nolan Johnston for the Human Monitoring Lab of Health Canada\n";
	file<<"c\nc\nc\nc ++++++++++++++++++++++++++++++++++++++++++++++++++++++\nc\nc      Cells\nc ++++++++++++++++++++++++++++++++++++++++++++++++++++++\nc\n"; 
	file<<"c Filling Universes\n";

	for (int headerCount=1; headerCount<=objectsAndVertices.groupNames.size(); headerCount++)
	{
		file<<"    "<<headerCount<<"   "<<headerCount<<"   "<<std::fixed<<std::setprecision(4)<<headerCount*-1<<"     -2 u = "<<headerCount<<
			"       $ "<<objectsAndVertices.groupNames[headerCount-1]<<"\n";
	}
	
	file<<"c\nc Lattice Unit Cell\nc\nc array dimensions: x-> 0:"<<modelRange.xRange<<"		y-> 0:"<<modelRange.yRange<<"		z-> 0:"<<modelRange.zRange<<"\n\n     ";

	bool init=false;
	fout.outputCount=0;
	//outputting the voxel itself
	for (int pCountX=0; pCountX<(modelRange.xRange+1); pCountX++)
	{
		for (int pCountY=0; pCountY<(modelRange.yRange+1); pCountY++)
		{
			for (int pCountZ=0; pCountZ<(modelRange.zRange+1); pCountZ++)
			{/*
				if (init==false)
				{
					outputInfo(fout, voxelArray, pCountX, pCountY, pCountZ);
					init=true;
				}

				if (fout.currentValue==voxelArray[pCountX][pCountY][pCountZ])
				{
					fout.numRep++;												//number of repititions, NOT number of identical voxel cells
				}
				else
				{
					if (fout.numRep==0)
					{
						file<<fout.currentValue<<" ";
						fout.currentColumns+=(fout.currentDigits+1);
						if (fout.currentColumns>20)
						{
							file<<"\n";
							fout.currentColumns=0;
						}
					}
					else
					{
						file<<fout.currentValue<<" "<<fout.numRep<<"r ";
						fout.currentColumns+=(fout.currentDigits+2);
						if (fout.currentColumns>20)
						{
							file<<"\n";
							fout.currentColumns=0;
						}
					}
					outputInfo(fout, voxelArray, pCountX, pCountY, pCountZ);
					fout.numRep=0;
				}*/
			

				// this block of code outputs the voxel in a block 16 columns wide, in a non-repeated structure
				if (fout.outputCount<16)
				{
					file<<voxelArray[pCountX][pCountY][pCountZ]<<" ";
					fout.outputCount++;
				}
				else
				{
					file<<"\n     ";
					file<<voxelArray[pCountX][pCountY][pCountZ]<<" ";
					fout.outputCount=1;
				}
				
			}
		}
	}



	file.close();
}
void outputInfo(OutputData& fout, int*** voxelArray, int pCountX, int pCountY, int pCountZ)
{
	fout.currentValue=voxelArray[pCountX][pCountY][pCountZ];
	int currValCopy=fout.currentValue;
	fout.currentDigits=0;

	if (currValCopy==0)
		fout.currentDigits=1;

	while (currValCopy!=0)
	{						
		currValCopy=currValCopy/10;
		fout.currentDigits++;
	}
}