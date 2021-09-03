<template>
  <v-container class="my-3 mx-auto">
    <v-row class="my-3">
      <v-toolbar dense flat>
        <v-btn small :disabled="photoidx<=0" @click="prevPhoto" dark color="primary"><v-icon>mdi-chevron-left</v-icon><span class="d-none d-md-block">Prev. Photo</span></v-btn>
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
        <v-btn small :disabled="photoidx>=(photos.length-1)" @click="nextPhoto" dark color="primary"><span class="d-none d-md-block">Next Photo</span><v-icon>mdi-chevron-right</v-icon></v-btn>
      </v-toolbar>
    </v-row>
    <v-row>
      <Photo v-model="photo" />
    </v-row>
  </v-container>
</template>

<script>
import DayPicker from '../components/DayPicker.vue';
import Photo from '../components/Photo'
import axios from 'axios';
import store from "../store";
import { AUTH_LOGOUT } from "../store/actions/auth";

export default {
  name: 'PhotoForm',
  components: {
    Photo,
    DayPicker
  },
  data: () => ({
    day: new Date().toISOString({ timeZone: 'Europe/Berlin' }).substr(0, 10),
    photos: [],
    photo: null,
    photoidx: null,
    authenticated: false,
    updateInterval: null
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
    loadDay: function(date) {
      let dateStr = date.toISOString({ timeZone: 'Europe/Berlin' }).substr(0, 10)
      axios.post(process.env['VUE_APP_BACKENDURL']+'/photos_per_day', { date: dateStr, token: store.getters.currentToken },
      ).then(response => {
        this.photos = response.data
        this.photoidx = this.photos.length - 1
        this.photo = this.photos[this.photoidx]
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        }
      });
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
  created() {
    this.updateInterval = setInterval(function() { this.loadDay(new Date(this.day)) }.bind(this), 30000)
  },
  destroyed() {
    clearInterval(this.updateInterval)
  },
  mounted () {
    // load photos
    this.loadDay(new Date(this.day))
  }
};
</script>

