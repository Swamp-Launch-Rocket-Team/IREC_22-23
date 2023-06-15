#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <unistd.h>


int main(int argc, char* argv[])
{
    auto take_photo = [argv]()
    {
        std::thread t([&]() {
            const std::filesystem::path relative_path_to_camera_script = "camera.py";

            // Get absolute path of camera python script
            std::filesystem::path path = argv[0];
            if(path.is_relative())
            {
                path = std::string(argv[0]).substr(2);
            }
            path = std::filesystem::absolute(path).parent_path();
            path = path / relative_path_to_camera_script;

            // Run camera python script
            std::string script = "python " + path.string() + " &";
            system(script.c_str());

            std::cout << "Thread finished executing file: \"" << path.string() << "\"" << std::endl;
        });
        t.detach();
        return t;
    };

    take_photo();
    sleep(1);

    return 0;
}
