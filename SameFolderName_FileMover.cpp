#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>

using namespace std;
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    string version = "0.1.1";
    string e_output = "Unknown Error";
    cout << "SameFolderName_FileMover " << "v" << version << endl;
    cout << "------------------------------" << endl;

    //引数指定されてなかったとき
    if (argc != 2) {
        cout << "SameFolderName_FileMover.exe [arg]" << endl;
        cout << "arg: [PATH]" << endl;
        cout << "Example: SameFolderName_FileMover.exe C:\\Users\\User\\Desktop\\Folder" << endl;
        return 2;
    } else {
        path folderPath = argv[1];

        if (!exists(folderPath)) {
            //パスなかった時
            e_output = "Path does not exist";
        } else if (!is_directory(folderPath)) {
            //ディレクトリじゃなかった時
            e_output = "Path is not a directory";
        } else {
            bool found = false;
            for (const auto& entry : directory_iterator(folderPath)) {
                if (is_directory(entry)) {
                    path subDir = entry.path();
                    int fileCount = 0;
                    path filePath;

                    //サブディレクトリ内のファイルを探してみる
                    for (const auto& subEntry : directory_iterator(subDir)) {
                        if (is_regular_file(subEntry)) {
                            //Zone.Identifierファイルは無視
                            if (subEntry.path().filename() == "$Zone.Identifier$$DATA") {
                                continue;

                                //思うんだけどこのファイルis何？
                            }
                            //ファイルが複数ある場合は無視
                            fileCount++;
                            filePath = subEntry.path();
                        }
                    }

                    //サブディレクトリのファイルが1つだけだった場合
                    if (fileCount == 1 && filePath.filename() == subDir.filename()) {

                        //Windowsの仕様でフォルダと同名のファイルがある場合、エラー出ちゃうのでtmp_movingに名前変更しちゃう
                        path tmpMovingPath = subDir.parent_path() / "tmp_moving";
                        rename(subDir, tmpMovingPath);

                        //ファイルを移動する
                        path newFilePath = folderPath / filePath.filename();
                        rename(tmpMovingPath / filePath.filename(), newFilePath);
                        cout << "Moved: " << filePath << " -> " << newFilePath << endl;


                        bool onlyZoneIdentifier = true;

                        //Zone.Identifierファイルだけだった場合はサブディレクトリを削除
                        //わざわざチェックする必要無いと思うんだけど、一応怖いから。
                        for (const auto& subEntry : directory_iterator(tmpMovingPath)) {
                            if (is_regular_file(subEntry) && subEntry.path().filename() != "$Zone.Identifier$$DATA") {
                                onlyZoneIdentifier = false;
                                break;
                            }
                        }
                        if (onlyZoneIdentifier) {
                            remove_all(tmpMovingPath);
                        }
                        //移動先に同じファイルがあったときに上書き確認処理入れようか考えたけど、
                        //よく考えたら「Windowsの仕様でフォルダと同名のファイルがある場合、エラー出ちゃう」仕様だから
                        //いらないよね？ってことで入れてない

                        found = true;
                    }
                }
            }
            if (!found) {
                cout << "No matching subdirectory found." << endl;
            }
            return 0;
        }
    }

    if (!e_output.empty()) {
        cerr << "Error: " << e_output << endl;
    }
    return 1;
}