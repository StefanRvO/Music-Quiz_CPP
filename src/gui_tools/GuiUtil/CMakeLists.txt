SET ( SRC_FILES
        ${SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/QuizSelector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TeamSelector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/QuizIntroScreen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/QuizWinningScreen.cpp
        CACHE INTERNAL ""
)

ADD_SUBDIRECTORY(ColorPicker)
ADD_SUBDIRECTORY(QExtensions)