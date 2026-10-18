#pragma once

class Program {
public:
    Program();
    bool loopActive() const;
    // Whether FrameStats should report (toggled with F3).
    bool showStats() const;

    friend class ProgramController;
private:
    bool _loopActive = true;
    bool _showStats = false;
};
