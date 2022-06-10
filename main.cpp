#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;
namespace fs = std::filesystem;

// Убирает лишние пробелы, либо добавляет один пробел после символа, определяющего строку
// Примеры:
//      "*   some text" --> "* some text"
//      "=>example.com text" --> "=> example.com text"
void FormatSpaces(string &buff)
{
    if(buff.empty()) return;

    int pos = 0; // pos - позиция, где должен стоять пробел после символа, определяющего строку
    switch (buff[0]) {
        case '#': {
            int count = 0;
            for (auto &ch: buff) {
                if (ch == '#') count++;
                else break;
            }
            pos = count;
            break;
        }
        case '*': {
            pos = 1;
            break;
        }
        case '>': {
            pos = 1;
            break;
        }
        case '=': {
            pos = 2;
            break;
        }
        default: {
            return;
        }
    }

    for(int i = pos; i < buff.size();) {
        if(buff[i] == buff[i+1] && buff[i] == ' ') buff.erase(pos, 1);
        else break;
    }
    if(buff[pos] != ' ') {
        buff.insert(pos, " ");
        return;
    }
}

// преобразует .gmi файл в .html
void ToHtml(const fs::path &file, const fs::path &out_directory)
{
    string filename = file.string();
    filename = filename.substr(filename.rfind('\\'));
    filename = filename.substr(0, filename.rfind("."));

    ofstream html_file(out_directory.string() + filename + ".html");
    ifstream original_file(file.string());

    html_file << "<html>\n";
    string buff;
    bool IsFirstUnList = true;
    bool IsPreformat = false;
    while (!original_file.eof()) {
        getline(original_file, buff);

        if (!IsFirstUnList && buff[0] != '*') {
            html_file << "</ul>\n";
            IsFirstUnList = true;
        }
        if(IsPreformat && buff[0] != '`') {
            html_file << buff << "\n";
            continue;
        }

        FormatSpaces(buff);
        switch (buff[0]) {
            case '#': {
                int count = 0;
                for (auto &ch: buff) {
                    if (ch == '#') count++;
                    else break;
                }
                html_file << "<h" << count << ">";
                html_file << buff.substr(count + 1) << "</h" << count << ">\n";
                break;
            }
            case '*': {
                if (IsFirstUnList) {
                    html_file << "<ul>\n";
                    IsFirstUnList = false;
                }
                html_file << "<li>" << buff.substr(2) << "</li>\n";
                break;
            }
            case '>': {
                html_file << "<blockquote>" << buff.substr(2) << "</blockquote>\n";
                break;
            }
            case '=': {
                html_file << "<p>\n<a href=\"";
                int i;
                for (i = 4; i < buff.size(); ++i) {
                    if (buff[i] == ' ') break;
                }
                if(i >= buff.size()) i = buff.size() - 1;
                html_file << buff.substr(3, i - 3) << "\">";
                html_file << buff.substr(i + 1) << "</a>\n</p>\n";
                break;
            }
            case '`': {
                if(!IsPreformat) {
                    html_file << "<pre>\n";
                    IsPreformat = true;
                } else {
                    html_file << "</pre>\n";
                    IsPreformat = false;
                }
                break;
            }
            default: {
                if(buff.empty()) html_file << "<p>&nbsp;</p>\n";
                else html_file << "<p>" << buff << "</p>\n";
                break;
            }
        }
    }
    html_file << "</html";

    html_file.close();
    original_file.close();
}

// функция для обхода директории
void pass(const fs::path &curr_path, const fs::path &out_directory)
{
    for (const auto &file : fs::directory_iterator(curr_path)) {
        if (fs::is_directory(file)) {
            string curr_folder_name = file.path().string();
            curr_folder_name = curr_folder_name.substr(curr_folder_name.rfind('\\'));
            fs::create_directory(out_directory.string() + curr_folder_name);
            pass(file.path(), out_directory.string() + curr_folder_name);
        } else {
            string path = file.path().string();
            if (path.rfind(".gmi") == string::npos) {
                fs::copy(file, out_directory);
            } else {
                ToHtml(file, out_directory);
            }
        }
    }
}

int main()
{
    string input_directory, output_directory;

    cout << "Enter the full path to the input_directory:";
    cin >> input_directory;
    cout << "Enter the full path to the output_directory:";
    cin >> output_directory;

    pass(input_directory, output_directory);

    return 0;
}
