/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ['index.html', 'play.html'],
  theme: {
    extend: {},
  },
  plugins: [
    require("@tailwindcss/typography"),
    require("daisyui")
  ],
}

