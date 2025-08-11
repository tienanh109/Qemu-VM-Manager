# Yêu cầu phiên bản CMake tối thiểu
cmake_minimum_required(VERSION 3.8)

# Tên dự án và ngôn ngữ
project(QemuGuiManager LANGUAGES CXX)

# Thiết lập tiêu chuẩn C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Thêm tệp thực thi chính của ứng dụng.
# Cờ WIN32 quan trọng để tạo ứng dụng GUI không có cửa sổ console.
add_executable(QemuGuiManager WIN32
    src/main.cpp
)

# Liên kết với các thư viện Windows cần thiết cho giao diện và hệ thống
target_link_libraries(QemuGuiManager PRIVATE user32 gdi32 shell32 comctl32 shlwapi)