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
  safelist: [
    'btn-success',
    'input-error',
    'input-success',
    'text-error',
    'text-success'
  ]
}

