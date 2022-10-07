//SPDX-FileCopyright: 2022 Makoto Yoshigoe
//SPDX-License-Identifier: GPL-3.0

#include<iostream>
#include<fstream>
#include<vector>
#include<iomanip>
#include<algorithm>
#include<math.h>
using std::vector;

constexpr int word_sum = 100, dimension = 15, default_frame = 40, TEMP = 0, TEST = 1;
vector<vector<std::string>> file_names(2, vector<std::string>(word_sum));
vector<vector<std::string>> words(2, vector<std::string>(word_sum));
vector<vector<double>> T(word_sum, vector<double>(word_sum));
int frame_data[2][word_sum];
//[テンプレートor未知データ][単語番号][フレーム][次元]
vector<vector<vector<vector<double>>>> data(2, vector<vector<vector<double>>>(word_sum, vector<vector<double>>(default_frame, vector<double>(dimension))));
vector<vector<vector<vector<double>>>> G(word_sum, vector<vector<vector<double>>>(word_sum, vector<vector<double>>(default_frame, vector<double>(default_frame))));
void read_file(int, int, int);
void dp(double);
/*
コマンドライン引数: 4つ
./実行ファイル名 テンプレートの話者 回数 未知入力の話者 回数
(例)実行ファイル名(a.out), テンプレート(1人目の1回目), 未知入力(1人目の2回目): 
./a.out 1 1 1 2
*/
int main(int argc, char *argv[]){
    read_file(TEMP, atoi(argv[1]), atoi(argv[2])); //テンプレートデータ読み込み
    read_file(TEST, atoi(argv[3]), atoi(argv[4])); //未知データ読み込み
    std::cout << "Finished loading file" << std::endl;
    double weight = 2;
    dp(weight);
    return 0;
}

//ファイルを読み込む関数. 引数（type: テンプレートと未知入力のどちらか, speaker: 話者, n: 1回目と2回目のどちらか）
void read_file(int type, int speaker, int n){
    std::string file_name, file_path, folder_name("city0"+std::to_string(speaker)+std::to_string(n)); //フォルダ名
    
    for(int word_num=0; word_num<word_sum; ++word_num){
        std::stringstream file_word_n;
        file_word_n <<"_"<< std::setfill('0')<<std::setw(3) << std::right<< std::to_string(word_num+1)<<".txt"; //単語番号
        file_path = folder_name + "/" + folder_name + file_word_n.str(); //読み込みファイルパス

        std::ifstream input_file(file_path);
        input_file >> file_names[type][word_num]; //ファイル名読み込み
        input_file >> words[type][word_num]; //単語読み込み
        input_file >> frame_data[type][word_num]; //フレーム数読み込み

        data[type][word_num].resize(frame_data[type][word_num]); //配列の要素数変更
        for(int i=default_frame; i<data[type][word_num].size(); ++i) data[type][word_num][i].resize(dimension);

        for(int i=0; i<frame_data[type][word_num]; ++i){
            for(int j=0; j<dimension; ++j){
                input_file >> data[type][word_num][i][j];
            }
        }
        input_file.close();
    }
}

//DPマッチングを行う関数. 引数（weight: 斜め遷移の際の重み）
void dp(double weight){
    int correct = 0;
    for(int w1=0; w1<word_sum; ++w1){
        for(int w2=0; w2<word_sum; ++w2){
            vector<vector<double>> d(frame_data[TEMP][w1], vector<double>(frame_data[TEST][w2])); //局所距離
            vector<vector<double>> g(frame_data[TEMP][w1], vector<double>(frame_data[TEST][w2])); //累積距離
            for(int i=0; i<frame_data[TEMP][w1]; ++i){
                for(int j=0; j<frame_data[TEST][w2]; ++j){
                    for(int k=0; k<dimension; ++k){
                        d[i][j] += (data[TEMP][w1][i][k]-data[TEST][w2][j][k])*(data[TEMP][w1][i][k]-data[TEST][w2][j][k]);
                    }
                    sqrt(d[i][j]);
                    if(i !=0 && j != 0){
                        vector<double> p = {g[i][j-1] + d[i][j], g[i-1][j-1] + weight*d[i][j], g[i-1][j] + d[i][j]};
                        g[i][j] = *std::min_element(p.begin(), p.end());
                    }
                    else if(i == 0) g[0][j] = g[0][j-1] + d[0][j];
                    else if(j == 0) g[i][0] = g[i-1][0] + d[i][0];
                    else g[0][0] = d[0][0];
                }
            }
            T[w1][w2] = g[frame_data[TEMP][w1]-1][frame_data[TEST][w2]-1] / (frame_data[TEMP][w1]+frame_data[TEST][w2]);
        }
        correct += (*std::min_element(T[w1].begin(), T[w1].end()) == T[w1][w1]);
    }
    std::cout << "weight =" << weight << ", Recognition rate: " << correct << "%"<< std::endl;
}
