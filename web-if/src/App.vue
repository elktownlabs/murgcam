<template>
  <v-app>
    <v-app-bar dense app v-if="authenticated">
      <v-app-bar-title><v-icon class="mx-3">mdi-camera-wireless</v-icon>Remote Webcam</v-app-bar-title>
      <v-spacer/>
      <v-btn @click="logout" dark color="red">Logout</v-btn>
    </v-app-bar>
    <v-main v-if="!authenticated">
      <Login @authenticated="onAuthenticated" />
    </v-main>
    <v-main v-if="authenticated">
      <v-container class="my-3 mx-auto">
      <v-row class="my-3">
          <v-toolbar dense flat>
            <v-btn small :disabled="photoidx<=0" @click="prevPhoto" dark color="indigo"><v-icon>mdi-chevron-left</v-icon><span class="d-none d-md-block">Prev. Photo</span></v-btn>
            <v-spacer/>
            <DayPicker ref="dayPicker" v-model="day" />
            <v-select v-model="photo" :items="photos" label="Time">
              <template slot="selection" slot-scope="data">
                {{ new Date(data.item.timestamp * 1000).toLocaleTimeString("en-US",  { timeZone: 'Europe/Berlin' }) }}
              </template>
              <template slot="item" slot-scope="data">
                {{ new Date(data.item.timestamp * 1000).toLocaleTimeString("en-US",  { timeZone: 'Europe/Berlin' }) }}
              </template>
            </v-select>
            <v-spacer/>
            <v-btn small :disabled="photoidx>=(photos.length-1)" @click="nextPhoto" dark color="indigo"><span class="d-none d-md-block">Next Photo</span><v-icon>mdi-chevron-right</v-icon></v-btn>
          </v-toolbar>
        </v-row>
        <v-row>
          <Photo v-model="photo" />
        </v-row>
      </v-container>
    </v-main>
  </v-app>
</template>

<script>
import DayPicker from './components/DayPicker.vue';
import Photo from './components/Photo'
import Login from './components/Login'
import axios from 'axios';

export default {
  name: 'App',
  components: {
    Photo,
    DayPicker,
    Login
  },
  data: () => ({
    day: new Date().toISOString({ timeZone: 'Europe/Berlin' }).substr(0, 10),
    photos: [],
    photo: null,
    photoidx: null,
    authenticated: false,
  }),
  methods: {
    nextPhoto: function() {
      if (this.photoidx < (this.photos.length - 1)) {
        this.photoidx++
        this.photo = this.photos[this.photoidx]
      }
    },
    prevPhoto: function() {
      if (this.photoidx > 0) {
        this.photoidx--
        this.photo = this.photos[this.photoidx]
      }
    },
    latestPhoto: function() {
      this.$refs.dayPicker.gotoLatest()
    },
    logout: function() {
      this.authenticated = false
      localStorage.authentication = null
    },
    onAuthenticated: function() {
      this.authenticated = true
      this.loadDay(new Date(this.day))
    },
    loadDay: function(date) {
      let authData = JSON.parse(localStorage.authentication)
      if (authData && authData.authenticated) {
        let dateStr = date.toISOString({ timeZone: 'Europe/Berlin' }).substr(0, 10)
        axios.get('https://wwv-schwarzwald.de/webcam/api/photos_per_day?date='+dateStr,
          { auth: {
            username: authData.user,
            password: authData.password
        }}).then(response => {
          this.photos = response.data
          this.photoidx = this.photos.length - 1
          this.photo = this.photos[this.photoidx]
        });
      }
    },
  },
  watch: {
    photo: function(val) {
      this.photoidx = this.photos.findIndex(data => data === val)
    },
    day: function(val) {
      if (val) {
        let today = new Date(val)
        this.loadDay(today)
      } else {
        this.photos = []
        this.photoidx = null
        this.photo = null
      }
    }
  },
  mounted () {
    let authData = JSON.parse(localStorage.authentication)
    if (authData && authData.authenticated) {
      this.authenticated = localStorage.authentication.authenticated
      // load photos
      this.loadDay(new Date(this.day))
    } else {
      this.authenticated = false
    }
  }
};
</script>
