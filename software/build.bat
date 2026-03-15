
rmdir /s /q Builds

JUCE\extras\Projucer\Builds\VisualStudio2022\x64\Release\App\Projucer.exe --resave main.jucer --ignore-global-settings
msbuild Builds\VisualStudio2022\AudioSettingsDemo.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m

Builds\VisualStudio2022\x64\Release\App\AudioSettingsDemo.exe