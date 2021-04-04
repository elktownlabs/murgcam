<template>
    <v-menu
        v-model="menu"
        :close-on-content-click="false"
        :nudge-right="40"
        transition="scale-transition"
        offset-y
        min-width="auto"
      >
        <template v-slot:activator="{ on, attrs }">
          <v-text-field
            v-model="date"
            label="Day"
            readonly
            v-bind="attrs"
            v-on="on"
          ></v-text-field>
        </template>
        <v-date-picker v-model="date" @input="menu=false" :picker-date.sync="pickerDate" event-color="green lighten-1" :events="daysWithPhotos" />
    </v-menu>
</template>
<script>
  import axios from 'axios';

  export default {
    name: "DayPicker",
    props: [ 'value' ],
    methods: {
      updateDate: function(val) {
        this.$emit('input', val)
      },
      loadDaysWithPictures: function(year, month) {
        let dateStr = "year="+year+"&month="+month
        axios.get('https://wwv-schwarzwald.de/webcam/api/photos_per_month?'+dateStr,
          { auth: {
            username: 'user',
            password: 'user00!'
        }}).then(response => {
          this.daysWithPhotos = []
          response.data.forEach(element => {
            this.daysWithPhotos.push(element.substr(0,10))
          })
        });
      }
    },
    watch: {
      pickerDate: function () {
        this.validDates = []
        let selectedMonth = new Date(this.pickerDate).getMonth() + 1
        let selectedYear = new Date(this.pickerDate).getFullYear()
        this.loadDaysWithPictures(selectedYear, selectedMonth)
      },
      date: function(val) {
        if (val) {
          this.updateDate(val)
        } else {
          this.updateDate(null)
        }
      },
      value: function(val) {
        console.log("X " + val)
      }
    },
    data: () => ({
      pickerDate: null,
      menu: false,
      daysWithPhotos: [],
      date: null
    }),
    mounted () {
      this.date = this.value
    },
  }
</script>