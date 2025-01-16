// Initialize and add the map
let map;

async function initMap() {
  // The location of Uluru
  const position = { lat: 37.0902, lng: -95.7129 };
  // Request needed libraries.
  //@ts-ignore
  const { Map } = await google.maps.importLibrary("maps");

  // The map, centered at Uluru
  map = new Map(document.getElementById("map"), {
    zoom: 4,
    center: position,
    mapId: "5aaecc8841ac4ca9",
    disableDefaultUI: true,
    // draggable: false, // Disable panning
    // scrollwheel: false, // Disable zooming with scroll wheel
    disableDoubleClickZoom: true, // Disable zooming with double click
  });

  // Fetch GeoJSON data
  const response = await fetch("../gz_2010_us_040_00_500k.json");
  const geoJsonData = await response.json();

  // Filter GeoJSON data for the state you need
  const stateNames = [
    "Washington", "California", "Nevada", "Idaho", "Utah", "Arizona", "Colorado", "South Dakota", "Oklahoma", "Texas", "Minnesota", "Iowa", 
    "Michigan", "Ohio", "Kentucky", "Tennessee", "Mississippi", "Georgia", "Florida", "Virginia", "Delaware", "New York", "Connecticut", "New Hapshire", "Maine"
  ];
  
  const stateFeatures = stateNames.map(stateName => 
    geoJsonData.features.find(feature => feature.properties.NAME === stateName)
  ).filter(feature => feature !== undefined); // Filter out undefined values

  const featureCollection = {
    type: "FeatureCollection",
    features: stateFeatures
  };

  // Add GeoJSON data to the map
  map.data.addGeoJson(featureCollection);

  // Style the GeoJSON data
  map.data.setStyle((feature) => {
    const stateName = feature.getProperty("NAME");
    let fillColor;

    if (["Washington", "Utah", "Michigan", "Mississippi", "Virginia", "Connecticut", "Maine"].includes(stateName)) {
      fillColor = "#73A641"; // green
    } else if (["California", "Idaho", "Colorado", "Texas", "Minnesota", "Kentucky", "Georgia", "New York"].includes(stateName)) {
      fillColor = "#F29B9B"; // pink
    } else if (["Nevada", "Iowa", "Oklahoma", "Tennessee", "Delaware"].includes(stateName)) {
        fillColor = "#F2C46D"; // yellow
    } else if (["Arizona","South Dakota", "Ohio", "New Hampshire", "Florida"].includes(stateName)) {
        fillColor = "#9B59B6";
    }

    return {
      fillColor: fillColor,
      fillOpacity: .9, // Set fill opacity to solid
      strokeWeight: 1,
      strokeColor: '#808080'
    };
  });
}

initMap();
