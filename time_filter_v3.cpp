/*
*	VERSAO 3: Detalhando os tempos QUADRO A QUADRO em que o canal está efetivamente ocupado
*	Programa para calcular o intervalo de tempo ocioso entre quadros
* 	> Argumento de entrada é o arquivo que será lido (gerado no seguinte formato) :
*		tshark -r arq_cap/cap01_14-55-47.pcapng -lnV|egrep 'delta from previous captured|\[Duration|\Frame Length:|\Data rate:|\Short preamble:' > dur.txt 
* 	> Arquivo de resultado: <command prompt>
*	> OBS:
*		1. compilar com : g++ -std=c++11 ffiltro_v3.cpp -o ffiltro_v3			
*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>

using namespace std;

struct frame
{
	double time_delta = NULL,
		   frame_length = NULL,
		   preamble = 0.000016,
		   data_rate = NULL,
		   nav = NULL,
		   frame_time = NULL;
};

int main(int argc, char *argv[])
{
	//Abrindo arquivo para leitura: argumento de linha de comando
	ifstream f_in;
	f_in.open(argv[1]);
	if (!f_in.good())
		return -1;

	//Abrindo arquivos de escrita:
	ofstream f_out;
	f_out.open("idle_times.txt");
	if (!f_out.good())
		return -1;

	string line;
	int pos = 0;
	int pos0 = 0;
	list<frame> frame_list;
	double time_delta_t = 0,
		   frame_length_t = 0,
		   preamble_t = 0,
		   nav_t = 0,
		   frame_time_t = 0,
		   radiotap_capture = 0;
	int negative_idle_time = 0;

	getline(f_in, line);
	while (line.length() > 0)
	{
		frame f0;
		//cout << "\n----------------------------------" << endl;

		//TIME DELTA
		pos = line.find("frame:");
		if (pos < line.length())
		{
			string time_delta = line.substr(pos + 7, 11);
			//cout << "time_delta: " << time_delta << " s" << endl;
			f0.time_delta = stod(time_delta);
			getline(f_in, line);
		}

		//FRAME LENGTH
		pos = line.find("Frame Length");
		if (pos < line.length())
		{
			pos = line.find("(") + 1;
			pos0 = line.find(" bits");
			string frame_length = line.substr(pos, pos0 - pos);
			//cout << "frame_length: " << frame_length << " bits" << endl;
			f0.frame_length = stod(frame_length);
			getline(f_in, line);
		}

		//PREAMBLE
		pos = line.find("preamble");
		if (pos < line.length())
		{
			pos = line.find(":") + 2;
			string preamble = line.substr(pos);
			//cout << "preamble: " << preamble << endl;
			if (preamble == "True")
				f0.preamble = 0.000008;
			getline(f_in, line);
		}

		//DATA RATE
		pos = line.find("Data rate");
		if (pos < line.length())
		{
			pos = line.find(":") + 2;
			pos0 = line.find(" Mb");
			string data_rate = line.substr(pos, pos0 - pos);
			//cout << "data_rate: " << data_rate << " Mbps" << endl;
			f0.data_rate = stod(data_rate);
			getline(f_in, line);
		}

		//NAV
		pos = line.find("Duration");
		if (pos < line.length())
		{
			pos = pos + 10;
			pos0 = line.find("µs");
			string nav = line.substr(pos, pos0 - pos);
			//cout << "nav: " << nav << " µs" << endl;
			f0.nav = stod(nav) * 0.000001;
			getline(f_in, line);
		}

		if (f0.data_rate == NULL)
			radiotap_capture++;
		else
		{
			//DELTA
			time_delta_t += f0.time_delta;

			//PREAMBLE
			preamble_t += f0.preamble;

			//FRAME LENGTH
			frame_length_t += f0.frame_length;
			f0.data_rate = f0.data_rate * 1000000;
			f0.frame_time = (f0.frame_length / f0.data_rate) + (16 / f0.data_rate); //TIME FRAME + SERVICE
			frame_time_t += f0.frame_time;
			//cout << "frame_time + service : " << f0.frame_time << endl;

			//NAV
			if (f0.nav)
				nav_t += f0.nav;

			if (frame_list.size() > 0)
			{
				frame f = frame_list.back();
				double idle = f0.time_delta - f.frame_time - f.preamble;
				if (idle >= 0)
					f_out << idle << endl;
				else
				{
					//f_out << "0" << endl;
					negative_idle_time++;
				}

				//f_out << "frame_length : " << f0.frame_length << "	|---------|    ";
				//f_out << "idle : " << idle << endl;
				//cout << "idle : " << idle << endl;
			}
		}

		frame_list.push_back(f0);
	}

	f_in.close();
	f_out.close();

	double idle_time = time_delta_t - frame_time_t - preamble_t;

	cout
		<< "\n----------------------------------\n"
		<< endl;
	cout << "time_delta_t 	  : " << time_delta_t << " s" << endl;
	cout << "frame_time_t 	  : " << frame_time_t << " s" << endl;
	cout << "preamble_t 	  : " << preamble_t << " s" << endl;
	cout << "nav_t 		  : " << nav_t << " s" << endl;
	//cout << "radiotap_capture  : " << radiotap_capture << " frames" << endl;
	cout << "idle_time	  : " << idle_time << " s" << endl;
	cout << "negative_idle_time : " << negative_idle_time << endl;
	cout << "\n----------------------------------\n"
		 << endl;

	cout << "tamanho da lista: " << frame_list.size() << endl;

	return 0;
}