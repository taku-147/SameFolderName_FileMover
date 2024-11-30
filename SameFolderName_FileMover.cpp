#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    string version = "1.0.0";
    string e_output = "Unknown Error";
    cout << "SameFolderName_FileMover " << "v" << version << endl;
    cout << "------------------------------" << endl;
    //引数指定されてなかったとき
    if (argc < 2) {
        cout << "SameFolderName_FileMover.exe [arg]" << endl;
        cout << "arg: [PATH]" << endl;
        cout << "Example: SameFolderName_FileMover.exe C:\\Users\\User\\Desktop\\Folder" << endl;
        return 2;
    } else {
        path folderPath = argv[1];
        if (!exists(folderPath)) {
            //パスなかった時
            e_output = "Path does not exist";
            return 1;
        } else if (!is_directory(folderPath)) {
            //ディレクトリじゃなかった時
            e_output = "Path is not a directory";
            return 1;
        } else {
            bool found = false;
            char ask;

            //聞いてみる
            cout << folderPath << endl << "Move files in subdirectories with the same name as the subdirectory to the parent directory? [y/n]: ";
            cin >> ask;
            if(ask != 'y') {
                return 0;
            }
            for (const auto& entry : directory_iterator(folderPath)) {
                if (is_directory(entry)) {
                    path subDir = entry.path();
                    int fileCount = 0;
                    path filePath;
                    //サブディレクトリ内のファイルを探してみる
                    for (const auto& subEntry : directory_iterator(subDir)) {
                        if (is_regular_file(subEntry)) {
                            //Zone.Identifierファイルは無視
                            if (subEntry.path().filename() == "$Zone.Identifier$$DATA" || subEntry.path().filename() == "$SmartScreen$$DATA") {
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
                        if (exists(tmpMovingPath)) {
                            remove_all(tmpMovingPath);
                        }
                        rename(subDir, tmpMovingPath);

                        //ファイルを移動する
                        path newFilePath = folderPath / filePath.filename();
                        rename(tmpMovingPath / filePath.filename(), newFilePath);
                        cout << "Moved: " << filePath << " -> " << newFilePath << endl;


                        bool onlyZoneIdentifier = true;

                        //Zone.Identifierファイルだけだった場合はサブディレクトリを削除
                        //わざわざチェックする必要無いと思うんだけど、一応怖いから。
                        for (const auto& subEntry : directory_iterator(tmpMovingPath)) {
                            if (is_regular_file(subEntry) && (subEntry.path().filename() != "$Zone.Identifier$$DATA" || subEntry.path().filename() != "$SmartScreen$$DATA")) {
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
        }
        cout << "------------------------------" << endl;
        int c = 0;

        //Zone.IdentifierファイルとSmartScreenファイルを削除
        for (const auto& entry : directory_iterator(folderPath)) {
            if (is_directory(entry)) {
                for (const auto& subEntry : directory_iterator(entry)) {
                    if (is_regular_file(subEntry)) {
                        path filePath = subEntry.path();
                        if (filePath.filename() == "$Zone.Identifier$$DATA" || filePath.filename() == "$SmartScreen$$DATA") {
                            remove(filePath);
                            cout << "Removed: " << filePath << endl;
                        }
                    }
                }
            }
        }

        //空のサブディレクトリを削除
        for (const auto& entry : directory_iterator(folderPath, directory_options::skip_permission_denied)) {
            if (is_directory(entry)) {
                for (const auto& subEntry : directory_iterator(entry)) {
                    if (is_directory(subEntry) && std::filesystem::is_empty(subEntry)) {
                        remove(subEntry);
                        cout << "Removed empty subdirectory: " << subEntry.path() << endl;
                    }
                }
            }
        }

        cout << "------------------------------" << endl;
        return 0;
    }
}