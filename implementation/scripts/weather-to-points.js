const fs = require("fs")
const path = require("path")

const DATE = "04-06-2024"
const DATE_FORMATTED = DATE.replaceAll("-", "%2F")  

const bins = [
  { lower: -200, upper: -50 },
  { lower: -50, upper: -40 },
  { lower: -30, upper: -20 },
  { lower: -20, upper: -10 },
  { lower: -10, upper: -5 },
  { lower: -5, upper: 0 },
  { lower: 0, upper: 2.5 },
  { lower: 2.5, upper: 5 },
  { lower: 5, upper: 7.5 },
  { lower: 7.5, upper: 10 },
  { lower: 10, upper: 12.5 },
  { lower: 12.5, upper: 15 },
  { lower: 15, upper: 17.5 },
  { lower: 17.5, upper: 20 },
  { lower: 20, upper: 22.5 },
  { lower: 22.5, upper: 25 },
  { lower: 25, upper: 27.5 },
  { lower: 27.5, upper: 30 },
  { lower: 30, upper: 35 },
  { lower: 35, upper: 40 },
  { lower: 40, upper: 50 },
  { lower: 50, upper: 200 },
]


const f = async () => {
  const res = await fetch(`https://wowapi.metweb.ie/getObservations/dry_bulb?date=${DATE_FORMATTED}&time=20&timePoint=-1&mapFilterTags=&showWowData=true&showOfficialData=true&showRegisteredSites=false`);
  const data = JSON.parse(await new Response(res.body).text())

  const lines = data.features.map(feat => {
    const temp = feat.properties.primary.dt
    if (temp == undefined) return "";
    const bin = bins.findIndex(x => x.lower <= temp && temp < x.upper)
    return `${feat.geometry.coordinates[0]};${feat.geometry.coordinates[1]};${bin}`
  })

  fs.writeFileSync(path.resolve(`../data/temperature-${DATE}.points`), lines.filter(x => x.length).join("\n"))
}

f();