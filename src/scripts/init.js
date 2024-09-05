// -- Handles the initialitzation of the chessboard and the standard C++ output into JavaScript console --


var Module = {
    onRuntimeInitialized: function() {
        Module.ccall('startGame'); // Starts a new game
        initializeChessboard();
    },
    print: (function() {
        var element = document.getElementById('output');
        if (element) element.value = ''; // clear browser cache
        return (...args) => {
            var text = args.join(' ');
            // These replacements are necessary if you render to raw HTML
            //text = text.replace(/&/g, "&amp;");
            //text = text.replace(/</g, "&lt;");
            //text = text.replace(/>/g, "&gt;");
            //text = text.replace('\n', '<br>', 'g');
            console.log(text);
            if (element) {
                element.value += text + "\n";
                element.scrollTop = element.scrollHeight;
            }
        };
    })(),
};