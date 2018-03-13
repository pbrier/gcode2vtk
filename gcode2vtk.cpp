//    GCode2vtk
// converts skeinforge gcode files to vtk files that can be visualized in e.g. paraview.
// usage: gcode2vtk vtk_filename <optinal output name>
// This software is hereby published using the GNU General Public License v3
// originally created by Bernhard Kubicek, 2011
// Removed "endl" output from inside the loops: they flush the output and make it dead slow!
// replaced with newline output ("\n") 


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <stdlib.h>

using namespace std;

vector<float> points;
vector<float> ext1;
vector<float> ext2;
vector<float> feedrate;


void skipwhite(istream &s)
{
	s.get();
	return;
	while(s.peek()==' ' || s.peek()=='\t')
	{
		char c=s.get();
		cout<<"skip:"<<c<<endl;
	}
}

void readpath(string ngc)
{
	fstream in(ngc.c_str(),fstream::in);
	if(!in.good())
	{
		cerr<<"Cannot open gcode file"<<endl;
		exit(1);
	}
	points.resize(0);
	points.reserve(100000);
	string tt,t;
	float x[]={0,0,0};
	float fr=0;
	float e1=0;
	points.push_back(0);points.push_back(0);points.push_back(0);
	//feedrate.push_back(0);
	ext1.push_back(0);
	while(in.good()) 
	{
		getline(in,tt);
		stringstream ins(tt);
		ins>>t;
		
		//cout<<"reading:"<<t<<endl;
		bool done;
		if(t=="G1" || t=="G0")
		{
			//x[0]=0;
			//x[1]=0;
			//x[2]=0;
			//cout<<"G1 found:"<<endl;
			
			while(1)
			{
			 done=false;
          skipwhite(ins);
          char c=ins.get();
          //cout<<"c="<<(c)<<endl;
          float tmp;
          ins>>tmp;
          switch(c)
          {
           case 'x':
           case 'X':
           	x[0]=tmp;
           break;
           case 'y':
           case 'Y':
           	x[1]=tmp;
           break;
           case 'z':
           case 'Z':
           	x[2]=tmp;
           break;
           case 'E':
             e1=tmp;
            break;
           case 'F':
           	fr=tmp;
            break;
           default:
          	done=true;
          	points.push_back(x[0]);
          	points.push_back(x[1]);
          	points.push_back(x[2]);
          	ext1.push_back(e1);
          	feedrate.push_back(fr);
          	//cout<<x[0]<<" "<<x[1]<<" "<<x[2]<<" "<<endl;
          
          }
          if(done) break;
         } //while not done
			//cout<<"end of g1"<<endl;
		} //g1 block
		else
		{
			//ignore line
		}
		
	
	}
	cout<<"Finished reading file"<<endl;
	
	feedrate.push_back(0);
	ext1.push_back(ext1.back());

}

void writeint(ostream &out,const int i)
{
	out.write((char*)&i,sizeof(int));
}

void writevtkdata(ostream &out,string name, int size, vector<float> data)
{
    cout<< "Write vtk data: " << name << ", size=" << size <<endl;
  
	out<<"SCALARS "<<name<<" float 1"<<endl;
	out<<"LOOKUP_TABLE default"<<endl;
	
	for(int i=0;i<size;i++)
	{
		out<<data[i]<<"\n"; // endl;
		//out.write((char*)&(data[i]),sizeof(float));
	}
	out<<endl;
	cout<< "Write vtk data done. " << endl << endl;
}
void writevtk(string name)
{
	fstream out(name.c_str(),fstream::out);
	if(!out.good())
	{
		cerr<<"Cannot write to output file"<<endl;
		exit(1);
	}	
	
	out<<"# vtk DataFile Version 2.0"<<endl;
	out<<name<<endl;
	out<<"ASCII"<<endl;
	//out<<"BINARY"<<endl;
	out<<"DATASET POLYDATA"<<endl;
	int n=points.size()/3;
	out<<"POINTS "<<n<<" float"<<endl;
	for(int i=0;i<points.size();i+=3)
	{
		out<<points[i+0]<<" "<<points[i+1]<<" "<<points[i+2]<<" \n"; // <<endl;
		//out.write((char*)&(points[0]),points.size()*sizeof(float));

	}
	out<<endl;
	out<<"LINES "<<n-1<<" "<<3*(n-1)<<endl;

	for(int i=0;i<n-1;i++)
	{
	 out<<"2 "<<i<<" "<<i+1<< "\n"; // endl;
	 //writeint(out,2);writeint(out,i);writeint(out,i+1);
	}
	out<<endl;
	out<<"CELL_DATA "<<(n-1)<<endl;
	writevtkdata(out,"filament1",n-1,ext1);
	writevtkdata(out,"feedrate",n-1,feedrate);
	
	
	vector<float> distances,segnr,time,eratio,seglen16; // extrusion ratio = e / |dx,dy,dz|
    float dist16[16];
	distances.resize(n);
	segnr.resize(n);
	time.resize(n);
	eratio.resize(n);
    seglen16.resize(n);
	
    for(int i=0; i<16; i++) 
      dist16[i] = 0.0;
    
	for(int i=0;i<n-1;i++)
	{
	    segnr[i]=i;
		distances[i]=sqrt(pow(points[i*3+3]-points[i*3+0],2)+\
								pow(points[i*3+4]-points[i*3+1],2)+\
								pow(points[i*3+5]-points[i*3+2],2));
        
	    float timeneeded=0;
	    if(feedrate[i]>0)
	      timeneeded=distances[i]/feedrate[i];
	    if(i==0)
	      time[i]=0;
	    else
	      time[i]=time[i-1]+timeneeded;
       
        // dist16[i & 0xF ] = 60.0*timeneeded; //  [mm/min] --> [mm/sec] 
        dist16[i & 0xF ] = distances[i]; // moving average distance of 16 segments        
        seglen16[i] = dist16[0] + dist16[1] + dist16[2] + dist16[3] + dist16[5] + dist16[6] + dist16[7] + 
          dist16[8] + dist16[9] + dist16[10] + dist16[11] + dist16[12] + dist16[13] + dist16[14] + dist16[15];
       
	    
	}
	
	writevtkdata(out,"seg_length",n-1,distances);
    writevtkdata(out,"buflen",n-1,seglen16);
	writevtkdata(out,"seg_number",n-1,segnr);
	writevtkdata(out,"build_time",n-1,time);
	vector<float> d;
	d.resize(n-1+1,0);
	for(int i=1;i<n-1;i++)
	{
		float thick=(ext1[i+1]-ext1[i]);
		
		if(distances[i]==0)
		  eratio[i] = 0;
		else
		  eratio[i] = thick / distances[i];
		
		if(distances[i]==0)
			thick=3;
		else
			thick*=1./distances[i];
		if(thick<0)thick=0;
		d[i]=thick;
	    
	}
	writevtkdata(out,"rel_d",n-1,d);
	writevtkdata(out,"eratio",n-1,eratio);
	
	out<<endl;
}

int main(int argc, char** argv)
{
	cout<<"Gcode2vtk converter"<<endl;
	if(argc<2)
	{
		cout<<"Please call with the name of the gcode file first."<<endl;
		cout<<"If a second command line option is given, it is the output filename."<<endl;
		return 1;
	}
	
	string ngc="";
	string vtk="";
	
	if(argc>=2)
		ngc=argv[1];
	
	if(argc>=3)
		vtk=argv[2];
	else
		vtk=ngc+".vtk";
	
	readpath(ngc);
	writevtk(vtk);

	
	
	return 0;
}
