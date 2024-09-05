// -- Handles the logic of the light and dark theme --


// Uses the localstorage or the system setting to set the theme
function calculateTheme({ localStorageTheme, systemSettingDark }) {
    return localStorageTheme || (systemSettingDark.matches ? "dark" : "light");
}

const localStorageTheme = localStorage.getItem("theme");
const systemSettingDark = window.matchMedia("(prefers-color-scheme: dark)");


// Set the initial theme
let currentTheme = calculateTheme({ localStorageTheme, systemSettingDark });
const themeToggle = document.getElementById("theme-toggle");
const themeIcon = document.getElementById("theme-icon");
themeToggle.checked = (currentTheme === "light");
document.documentElement.setAttribute("data-theme", currentTheme);


// Switches the theme
themeToggle.addEventListener("click", () => {
    currentTheme = (currentTheme === "dark") ? "light" : "dark";
    document.documentElement.setAttribute("data-theme", currentTheme);
    localStorage.setItem("theme", currentTheme);
});