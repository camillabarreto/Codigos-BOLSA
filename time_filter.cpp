/*
*	VERSAO 2: Detalhando os tempos em que o canal está efetivamente ocupado
*	Programa para calcular o intervalo de tempo ocioso entre quadros
* 	> Argumento de entrada é o arquivo que será lido (gerado no seguinte formato) :
*		tshark -r arq_cap/cap01_14-55-47.pcapng -lnV|egrep 'delta from previous captured|\[Duration|\Frame Length:|\Data rate:|\Short preamble:' > dur.txt 
* 	> Arquivo de resultado: <command prompt>
*	> OBS:
*		1. compilar com : g++ -std=c++11 ffiltro_v2.cpp -o ffiltro_v2			
*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

int main(int argc, char *argv[])
{

	//Abrindo arquivo para escrita: argumento de linha de comando
	ifstream f_in;
	f_in.open(argv[1]);
	if (!f_in.good())
		return -1;

	string line;
	int pos = 0;
	int pos0 = 0;
	double time_delta_t = 0,
		   frame_length_t = 0,
		   preamble_t = 0,
		   nav_t = 0,
		   frame_time_t = 0,
		   radiotap_capture = 0;

	getline(f_in, line);
	while (line.length() > 0)
	{
		double time_delta_t0 = NULL,
			   frame_length_t0 = NULL,
			   preamble_t0 = 0.000016,
			   data_rate_t0 = NULL,
			   nav_t0 = NULL,
			   frame_time_t0 = NULL;

		//TIME DELTA
		pos = line.find("frame:");
		if (pos < line.length())
		{
			string time_delta = line.substr(pos + 7, 11);
			//cout << "time_delta: " << time_delta << " s" << endl;
			time_delta_t0 = stod(time_delta);
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
			frame_length_t0 = stod(frame_length);
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
				preamble_t0 = 0.000008;
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
			data_rate_t0 = stod(data_rate);
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
			nav_t0 = stod(nav) * 0.000001;

			getline(f_in, line);
		}

		if (data_rate_t0 == NULL)
		{
			//cout << "RADIOTAP CAPTURE" << endl;
			radiotap_capture++;
		}
		else
		{
			//DELTA
			time_delta_t += time_delta_t0;

			//PREAMBLE
			preamble_t += preamble_t0;
			//cout << "preamble : " << preamble_t0 << endl;

			//FRAME LENGTH
			frame_length_t += frame_length_t0;
			data_rate_t0 = data_rate_t0 * 1000000;
			frame_time_t0 = (frame_length_t0 / data_rate_t0) + (16 / data_rate_t0); //TIME FRAME + SERVICE
			frame_time_t += frame_time_t0;
			//cout << "frame_time + service : " << frame_time_t0 << endl;

			//NAV
			if (nav_t0)
			{
				nav_t += nav_t0;
				//cout << "nav : " << (nav_t0) << endl;
			}

			//cout << "\n----------------------------------\n" << endl;
		}
	}

	f_in.close();

	double idle_time = time_delta_t - frame_time_t - preamble_t - nav_t;

	cout
		<< "\n----------------------------------\n"
		<< endl;
	cout << "time_delta_t 	  : " << time_delta_t << " s" << endl;
	cout << "frame_time_t 	  : " << frame_time_t << " s" << endl;
	cout << "preamble_t 	  : " << preamble_t << " s" << endl;
	cout << "nav_t 		  : " << nav_t << " s" << endl;
	cout << "radiotap_capture  : " << radiotap_capture << " frames" << endl;
	cout << "idle time	  : " << idle_time << " s" << endl;
	cout << "\n----------------------------------\n"
		 << endl;

	return 0;
}